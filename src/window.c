#include <string.h>
#include "window.h"
#include "utils.h"
#include "adb.h"
#include "app.h"

/* AppDetailsDialog Tanımları */
#define APP_TYPE_DETAILS_DIALOG (app_details_dialog_get_type ())
G_DECLARE_FINAL_TYPE (AppDetailsDialog, app_details_dialog, APP, DETAILS_DIALOG, AdwDialog)

struct _AppDetailsDialog
{
  AdwDialog parent_instance;

  GtkImage *app_icon;
  GtkLabel *app_name;
  GtkLabel *package_name;
  AdwActionRow *app_version;
  AdwActionRow *app_size;
  AdwActionRow *app_uid;
  AdwActionRow *install_date;
  GtkButton *uninstall_button;
  GtkButton *backup_button;
  GtkLabel *category_label;
  GtkStack *content_stack;

  AppInfo *app_info;
  gchar *serial; /* Cihaz seri numarası */
};

G_DEFINE_FINAL_TYPE (AppDetailsDialog, app_details_dialog, ADW_TYPE_DIALOG)

static void
app_details_dialog_finalize (GObject *object)
{
  AppDetailsDialog *self = APP_DETAILS_DIALOG (object);

  g_clear_object (&self->app_info);
  g_free (self->serial);

  G_OBJECT_CLASS (app_details_dialog_parent_class)->finalize (object);
}

static void
app_details_dialog_class_init (AppDetailsDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = app_details_dialog_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/com/muha/AppManager/ui/details_dialog.ui");

  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, app_icon);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, app_name);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, package_name);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, app_version);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, app_size);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, app_uid);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, install_date);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, uninstall_button);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, backup_button);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, category_label);
  gtk_widget_class_bind_template_child (widget_class, AppDetailsDialog, content_stack);
}

static void
app_details_dialog_init (AppDetailsDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}



static void
on_details_loaded (GObject      *source_object G_GNUC_UNUSED,
                   GAsyncResult *result,
                   gpointer      user_data)
{
  AppDetailsDialog *self = APP_DETAILS_DIALOG (user_data);
  GError *error = NULL;
  AdbPackageDetails *details;

  details = adb_get_package_details_finish (result, &error);

  if (error)
    {
      g_warning ("Detaylar alınamadı: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      /* AppInfo güncelle */
      if (details->version) app_info_set_version (self->app_info, details->version);
      if (details->size) app_info_set_size (self->app_info, details->size);
      if (details->uid) app_info_set_uid (self->app_info, details->uid);
      if (details->install_date) app_info_set_install_date (self->app_info, details->install_date);

      /* UI güncelle */
      adw_action_row_set_subtitle (self->app_version, app_info_get_version (self->app_info) ? app_info_get_version (self->app_info) : "-");
      adw_action_row_set_subtitle (self->app_size, app_info_get_size (self->app_info) ? app_info_get_size (self->app_info) : "-");
      adw_action_row_set_subtitle (self->app_uid, app_info_get_uid (self->app_info) ? app_info_get_uid (self->app_info) : "-");
      adw_action_row_set_subtitle (self->install_date, app_info_get_install_date (self->app_info) ? app_info_get_install_date (self->app_info) : "-");

      adb_package_details_free (details);
    }

  /* Yükleme bitti, detayları göster */
  if (self->content_stack)
    gtk_stack_set_visible_child_name (self->content_stack, "details");

  /* Dialog referansını bırak (callback user_data) */
  g_object_unref (self);
}

static AppDetailsDialog *
app_details_dialog_new (AppInfo *app, const gchar *serial)
{
  AppDetailsDialog *self = g_object_new (APP_TYPE_DETAILS_DIALOG, NULL);
  self->app_info = g_object_ref (app);
  self->serial = g_strdup (serial);

  /* Widget Kontrolleri */
  if (!self->app_name || !self->package_name || !self->app_version ||
      !self->app_size || !self->app_uid || !self->install_date || !self->category_label)
    {
      g_warning ("app_details_dialog_new: CRITICAL: One or more template children are NULL!");
    }

  gtk_label_set_text (self->app_name, app_info_get_label (app));
  gtk_label_set_text (self->package_name, app_info_get_package_name (app));

  /* Yükleme ekranını göster */
  if (self->content_stack)
    gtk_stack_set_visible_child_name (self->content_stack, "loading");

  /* Detayları ADB'den çek (Asenkron) */

  /* Dialog'un yaşam döngüsünü korumak için ref alıyoruz, callback'te unref yapacağız */
  g_object_ref (self);
  adb_get_package_details_async (serial, app_info_get_package_name (app), NULL, on_details_loaded, self);

  /* Kategori Etiketi Ayarları */
  if (self->category_label == NULL)
    {
      g_warning ("app_details_dialog_new: category_label is NULL! Template binding failed?");
    }
  else
    {
      AppCategory category = app_info_get_category (app);
      const gchar *cat_label = "Bilinmeyen";
      if (category == APP_CATEGORY_MALICIOUS) cat_label = "Zararlı";
      else if (category == APP_CATEGORY_SAFE) cat_label = "Güvenli";

      gtk_label_set_text (self->category_label, cat_label);
    }

  return self;
}

static void load_applications (MainWindow *self, const gchar *serial);
static void on_app_toggled (GtkCheckButton *check, gpointer user_data);
static void on_details_clicked (GtkButton *btn, gpointer user_data);

struct _MainWindow
{
  AdwApplicationWindow parent_instance;

  GtkDropDown *device_dropdown;
  GtkStack *main_stack;
  GtkStack *content_stack;
  GtkDropDown *sort_dropdown;

  GtkStack *unknown_stack;
  GtkStack *malicious_stack;
  GtkStack *safe_stack;

  GtkListBox *unknown_list;
  GtkListBox *malicious_list;
  GtkListBox *safe_list;
  GtkListBox *all_list;

  AdwSplitButton *select_button;

  GtkLabel *selected_label;
  GtkLabel *malicious_label;
  GtkLabel *safe_label;
  GtkLabel *total_label;

  GtkToggleButton *search_toggle;
  GtkButton *remove_button;
  GtkSearchEntry *search_entry;
  GtkBox *search_filters_box;
  GtkToggleButton *filter_app_name;
  GtkToggleButton *filter_package_id;
  /* Sıralama Düğmeleri - Şimdilik işlevsiz */
  GtkButton *filter_size;
  GtkButton *filter_date;

  gulong device_handler_id;
};

G_DEFINE_FINAL_TYPE (MainWindow, main_window, ADW_TYPE_APPLICATION_WINDOW)

static void
main_window_class_init (MainWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/muha/AppManager/ui/window.ui");

  gtk_widget_class_bind_template_child (widget_class, MainWindow, device_dropdown);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, main_stack);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, content_stack);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, sort_dropdown);

  gtk_widget_class_bind_template_child (widget_class, MainWindow, unknown_stack);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, malicious_stack);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, safe_stack);

  gtk_widget_class_bind_template_child (widget_class, MainWindow, unknown_list);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, malicious_list);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, safe_list);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, all_list);

  gtk_widget_class_bind_template_child (widget_class, MainWindow, select_button);

  gtk_widget_class_bind_template_child (widget_class, MainWindow, selected_label);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, malicious_label);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, safe_label);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, total_label);

  gtk_widget_class_bind_template_child (widget_class, MainWindow, search_toggle);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, remove_button);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, search_entry);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, search_filters_box);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, filter_app_name);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, filter_package_id);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, filter_size);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, filter_date);
}

static void on_device_selected (GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data);

static void
on_details_clicked (GtkButton *btn G_GNUC_UNUSED,
                    gpointer   user_data)
{
  AdwActionRow *row = ADW_ACTION_ROW (user_data);
  MainWindow *self = MAIN_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (row), MAIN_TYPE_WINDOW));
  AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

  if (!app || !self)
    {
      g_warning ("on_details_clicked: app veya self NULL");
      return;
    }

  g_debug ("on_details_clicked: Row clicked for app %s", app_info_get_package_name (app));

  /* Serial'ı bul */
  /* Not: Serial'ı MainWindow'da saklamak daha iyi olurdu ama şimdilik dropdown'dan tekrar parse edelim veya
     load_applications çağrıldığında saklanan bir serial değişkeni kullanalım.
     MainWindow yapısına 'current_serial' eklemek en temizidir ama şimdilik dropdown'dan alalım. */

  /* Dropdown'dan serial alma mantığı on_device_selected içinde var, bunu bir helper yapabiliriz.
     Veya on_device_selected çalıştığında serial'ı self->current_serial'a kaydedebiliriz.
     Şimdilik hızlı çözüm: Dropdown modelinden al. */

  GtkStringList *model = GTK_STRING_LIST (gtk_drop_down_get_model (self->device_dropdown));
  if (!model)
    {
      g_warning ("on_details_clicked: Model NULL");
      return;
    }

  guint selected_index = gtk_drop_down_get_selected (self->device_dropdown);
  const gchar *selected_string = gtk_string_list_get_string (model, selected_index);
  gchar *serial = NULL;

  if (selected_string)
    {
       /* Basit parse: "Model (Serial)" */
       gchar *temp_string = g_strdup (selected_string);
       char *open_paren = strrchr (temp_string, '(');
       char *close_paren = strrchr (temp_string, ')');

       if (open_paren && close_paren && close_paren > open_paren)
         {
            *close_paren = '\0';
            serial = g_strdup (open_paren + 1);
         }
       g_free (temp_string);
    }

  if (serial)
    {
      g_debug ("on_details_clicked: Serial found: %s", serial);
      AppDetailsDialog *dialog = app_details_dialog_new (app, serial);
      adw_dialog_present (ADW_DIALOG (dialog), GTK_WIDGET (self));

      /* Dialog kapandığında listeyi yenilemek gerekebilir eğer kategori değiştiyse.
         Şimdilik basit bırakalım. */

      g_free (serial);
    }
  else
    {
      g_warning ("on_details_clicked: Serial not found");
    }
}

static void
on_devices_loaded (GObject      *source_object G_GNUC_UNUSED,
                   GAsyncResult *result,
                   gpointer      user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  GList *devices;
  GError *error = NULL;
  GtkStringList *string_list;

  devices = adb_get_devices_finish (result, &error);

  /* Mevcut listeyi temizle (basitçe yeni liste oluşturarak) */
  string_list = gtk_string_list_new (NULL);

  if (error)
    {
      g_warning ("ADB hatası: %s", error->message);
      g_error_free (error);
      gtk_stack_set_visible_child_name (self->main_stack, "no-device");
      /* TODO: Hata mesajını arayüzde göster */
      gtk_drop_down_set_model (self->device_dropdown, NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (self->device_dropdown), FALSE);
      g_object_unref (string_list);
    }
  else if (devices == NULL)
    {
      gtk_stack_set_visible_child_name (self->main_stack, "no-device");
      gtk_drop_down_set_model (self->device_dropdown, NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (self->device_dropdown), FALSE);
      g_object_unref (string_list);
    }
  else
    {
      GList *l;
      guint n_devices = 0;

      for (l = devices; l != NULL; l = l->next)
        {
          AdbDevice *device = l->data;
          gchar *label;

          if (device->is_authorized)
            label = g_strdup_printf ("%s (%s)", device->model, device->serial);
          else
            label = g_strdup_printf ("%s (%s) (Yetkisiz)", device->model, device->serial);

          gtk_string_list_append (string_list, label);
          g_free (label);
          n_devices++;
        }
      /* AdbDevice yapılarını ve listeyi temizle */
      /* Not: adb_device_free fonksiyonu adb.h'da tanımlı */
      g_list_free_full (devices, (GDestroyNotify) adb_device_free);

      if (n_devices > 0)
        {
          /* Model değiştiğinde sinyal tetiklenir, bunu engelle */
          g_signal_handler_block (self->device_dropdown, self->device_handler_id);
          gtk_drop_down_set_model (self->device_dropdown, G_LIST_MODEL (string_list));
          g_signal_handler_unblock (self->device_dropdown, self->device_handler_id);

          gtk_widget_set_sensitive (GTK_WIDGET (self->device_dropdown), TRUE);

          /* Model değiştiğinde sinyal tetiklenmeyebilir (örn. index 0 -> 0),
             bu yüzden manuel olarak durumu güncelle. */
          on_device_selected (self->device_dropdown, NULL, self);
        }
      else
        {
          gtk_drop_down_set_model (self->device_dropdown, NULL); /* Modeli temizle */
          gtk_widget_set_sensitive (GTK_WIDGET (self->device_dropdown), FALSE);
          gtk_stack_set_visible_child_name (self->main_stack, "no-device");
        }

      g_object_unref (string_list);
    }
}

static void
refresh_devices (MainWindow *self)
{
  adb_get_devices_async (NULL, on_devices_loaded, self);
}

static void on_app_toggled (GtkCheckButton *check, gpointer user_data);

static void
sync_selection_for_app (MainWindow *self, AppInfo *app)
{
  GtkListBox *lists[] = {self->all_list, self->unknown_list, self->malicious_list, self->safe_list};

  for (int i = 0; i < 4; i++)
    {
      if (!lists[i]) continue;

      GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (lists[i]));
      while (child)
        {
          AdwActionRow *row = ADW_ACTION_ROW (child);
          AppInfo *row_app = g_object_get_data (G_OBJECT (row), "app-info");

          if (row_app == app)
            {
              GtkWidget *prefix = g_object_get_data (G_OBJECT (row), "selection-checkbox");
              if (GTK_IS_CHECK_BUTTON (prefix))
                {
                  gulong handler_id = (gulong)g_object_get_data (G_OBJECT (prefix), "handler-id");
                  if (handler_id > 0)
                    {
                      g_signal_handler_block (prefix, handler_id);
                      gtk_check_button_set_active (GTK_CHECK_BUTTON (prefix), app->is_selected);
                      g_signal_handler_unblock (prefix, handler_id);
                    }
                }
              break;
            }

          child = gtk_widget_get_next_sibling (child);
        }
    }
}

static void
update_button_labels (MainWindow *self)
{
  const gchar *visible_child = gtk_stack_get_visible_child_name (self->content_stack);
  GtkListBox *current_list = NULL;
  gboolean any_selected_global = FALSE;
  gboolean any_selected_in_category = FALSE;
  GtkWidget *child;

  /* Önce herhangi bir uygulamanın seçili olup olmadığını kontrol et (Global) */
  for (child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app && app->is_selected)
        {
          any_selected_global = TRUE;
          break;
        }
    }

  if (g_str_equal (visible_child, "all"))
    {
      current_list = self->all_list;
      any_selected_in_category = any_selected_global;
    }
  else if (g_str_equal (visible_child, "safe"))
    current_list = self->safe_list;
  else if (g_str_equal (visible_child, "malicious"))
    current_list = self->malicious_list;
  else if (g_str_equal (visible_child, "unknown"))
    current_list = self->unknown_list;

  /* Mevcut kategoride seçili öge var mı? */
  if (current_list && !any_selected_in_category)
    {
      for (child = gtk_widget_get_first_child (GTK_WIDGET (current_list));
           child != NULL;
           child = gtk_widget_get_next_sibling (child))
        {
          AdwActionRow *row = ADW_ACTION_ROW (child);
          AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

          if (app && app->is_selected)
            {
              any_selected_in_category = TRUE;
              break;
            }
        }
    }

  /* Ana düğmeyi (SplitButton) güncelle */
  if (self->select_button)
    {
      if (any_selected_in_category)
        {
          adw_split_button_set_label (self->select_button, "Seçimi Temizle");
          gtk_widget_set_tooltip_text (GTK_WIDGET (self->select_button), "Seçimi temizle (Ctrl+A)");
          adw_split_button_set_icon_name (self->select_button, "edit-clear-all-symbolic");
        }
      else
        {
          adw_split_button_set_label (self->select_button, "Tümünü Seç");
          gtk_widget_set_tooltip_text (GTK_WIDGET (self->select_button), "Tümünü seç (Ctrl+A)");
          adw_split_button_set_icon_name (self->select_button, "edit-select-all-symbolic");
        }
    }

  /* Menüdeki "Herşeyi Seç" eylemini güncelle */
  const gchar *label;
  if (any_selected_global)
    label = "Tüm Seçimi Temizle (Ctrl+Shift+A)";
  else
    label = "Herşeyi Seç (Ctrl+Shift+A)";

  GMenu *menu = g_menu_new ();
  g_menu_append (menu, label, "win.select-all-global");
  adw_split_button_set_menu_model (self->select_button, G_MENU_MODEL (menu));
  g_object_unref (menu);
}

static void
update_selection_status (MainWindow *self)
{
  GtkWidget *child;
  guint selected_count = 0;
  guint total_count = 0;
  guint malicious_count = 0;
  guint safe_count = 0;

  /* Tüm listeden sayıyoruz (her uygulama sadece bir kez sayılmalı) */
  for (child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app)
        {
          total_count++;

          if (app->is_selected)
            selected_count++;

          AppCategory category = app_info_get_category (app);
          if (category == APP_CATEGORY_MALICIOUS)
            malicious_count++;
          else if (category == APP_CATEGORY_SAFE)
            safe_count++;
        }
    }

  g_autofree gchar *selected_text = g_strdup_printf ("Seçili: %u", selected_count);
  g_autofree gchar *malicious_text = g_strdup_printf ("Zararlı: %u", malicious_count);
  g_autofree gchar *safe_text = g_strdup_printf ("Güvenli: %u", safe_count);
  g_autofree gchar *total_text = g_strdup_printf ("Toplam: %u", total_count);

  gtk_label_set_text (self->selected_label, selected_text);
  gtk_label_set_text (self->malicious_label, malicious_text);
  gtk_label_set_text (self->safe_label, safe_text);
  gtk_label_set_text (self->total_label, total_text);

  /* Kaldır düğmesini seçili uygulama varsa etkinleştir, yoksa devre dışı bırak */
  if (self->remove_button)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (self->remove_button), selected_count > 0);
    }

  update_button_labels (self);
}

static void
on_select_all_action (GSimpleAction *action G_GNUC_UNUSED,
                      GVariant      *parameter G_GNUC_UNUSED,
                      gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  const gchar *visible_child = gtk_stack_get_visible_child_name (self->content_stack);
  GtkListBox *current_list = NULL;

  if (g_str_equal (visible_child, "all"))
    current_list = self->all_list;
  else if (g_str_equal (visible_child, "safe"))
    current_list = self->safe_list;
  else if (g_str_equal (visible_child, "malicious"))
    current_list = self->malicious_list;
  else if (g_str_equal (visible_child, "unknown"))
    current_list = self->unknown_list;

  if (!current_list)
    return;

  GtkWidget *child;
  gboolean any_selected_in_category = FALSE;

  /* Mevcut kategoride seçili öge var mı diye kontrol et */
  for (child = gtk_widget_get_first_child (GTK_WIDGET (current_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app && app->is_selected)
        {
          any_selected_in_category = TRUE;
          break;
        }
    }

  /* Duruma göre işlem yap: Eğer herhangi biri seçiliyse temizle, değilse hepsini seç */
  gboolean new_state = !any_selected_in_category;

  for (child = gtk_widget_get_first_child (GTK_WIDGET (current_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app)
        {
          app->is_selected = new_state;
          sync_selection_for_app (self, app);
        }
    }

  update_selection_status (self);
}

static void
on_select_all_global_action (GSimpleAction *action G_GNUC_UNUSED,
                              GVariant      *parameter G_GNUC_UNUSED,
                              gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  GtkWidget *child;
  gboolean any_selected = FALSE;

  /* Önce herhangi bir uygulamanın seçili olup olmadığını kontrol et */
  for (child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app && app->is_selected)
        {
          any_selected = TRUE;
          break;
        }
    }

  /* Duruma göre işlem yap: Eğer herhangi biri seçiliyse hepsini temizle, değilse hepsini seç */
  gboolean new_state = !any_selected;

  for (child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app)
        {
          app->is_selected = new_state;
          sync_selection_for_app (self, app);
        }
    }

  update_selection_status (self);
}

static void
on_refresh_action (GSimpleAction *action G_GNUC_UNUSED,
                   GVariant      *parameter G_GNUC_UNUSED,
                   gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  utils_reload_app_data ();
  refresh_devices (self);
}

static void
on_category_changed (GSimpleAction *action G_GNUC_UNUSED,
                     GVariant      *parameter,
                     gpointer       user_data)
{
  AdwActionRow *row = ADW_ACTION_ROW (user_data);
  MainWindow *self = MAIN_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (row), MAIN_TYPE_WINDOW));
  AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

  if (!app || !self)
    return;

  const gchar *category_str = g_variant_get_string (parameter, NULL);
  AppCategory old_category = app_info_get_category (app);
  AppCategory new_category = APP_CATEGORY_UNKNOWN;

  if (g_str_equal (category_str, "malicious"))
    new_category = APP_CATEGORY_MALICIOUS;
  else if (g_str_equal (category_str, "safe"))
    new_category = APP_CATEGORY_SAFE;
  else if (g_str_equal (category_str, "unknown"))
    new_category = APP_CATEGORY_UNKNOWN;

  if (old_category == new_category)
    return;

  /* Kategoriyi güncelle */
  app_info_set_category (app, new_category);
  utils_save_app_category (app_info_get_package_name (app), old_category, new_category);

  /* Tüm listelerdeki bu satırları bul ve sil */
  GtkListBox *lists[] = {self->all_list, self->unknown_list, self->malicious_list, self->safe_list};

  for (int i = 0; i < 4; i++)
    {
      GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (lists[i]));
      while (child)
        {
          GtkWidget *next = gtk_widget_get_next_sibling (child);
          AppInfo *row_app = g_object_get_data (G_OBJECT (child), "app-info");

          if (row_app && g_str_equal (app_info_get_package_name (row_app), app_info_get_package_name (app)))
            {
              gtk_list_box_remove (lists[i], child);
            }

          child = next;
        }
    }

  /* Yeni kategoriye satır ekle */
  GtkListBox *target_lists[2] = {
    self->all_list,
    NULL
  };

  if (new_category == APP_CATEGORY_UNKNOWN)
    target_lists[1] = self->unknown_list;
  else if (new_category == APP_CATEGORY_MALICIOUS)
    target_lists[1] = self->malicious_list;
  else if (new_category == APP_CATEGORY_SAFE)
    target_lists[1] = self->safe_list;

  for (int i = 0; i < 2; i++)
    {
      if (target_lists[i] == NULL)
        continue;

      /* Yeni satır oluştur */
      AdwActionRow *new_row = ADW_ACTION_ROW (adw_action_row_new ());
      g_object_set_data_full (G_OBJECT (new_row), "app-info", g_object_ref (app), g_object_unref);

      /* Checkbox ekle (En sol) */
      GtkWidget *check = gtk_check_button_new ();
      if (app->is_selected)
        gtk_check_button_set_active (GTK_CHECK_BUTTON (check), TRUE);
      if (new_category == APP_CATEGORY_MALICIOUS && !app->is_selected)
        {
          gtk_check_button_set_active (GTK_CHECK_BUTTON (check), TRUE);
          app->is_selected = TRUE;
        }
      gulong handler_id = g_signal_connect (check, "toggled", G_CALLBACK (on_app_toggled), self);
      g_object_set_data (G_OBJECT (check), "handler-id", (gpointer)handler_id);
      adw_action_row_add_prefix (new_row, check);

      /* Checkbox'ı sakla */
      g_object_set_data (G_OBJECT (new_row), "selection-checkbox", check);

      /* Aktivasyon widget'ı olarak ayarla */
      adw_action_row_set_activatable_widget (new_row, check);

      /* Kategori değiştirme düğmesi (Checkbox'ın yanı) */
      GtkWidget *category_button = gtk_menu_button_new ();
      gtk_widget_set_valign (category_button, GTK_ALIGN_CENTER);
      gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (category_button), "view-list-symbolic");
      gtk_widget_add_css_class (category_button, "flat");
      gtk_widget_set_tooltip_text (category_button, "Kategoriyi Değiştir");

      /* Aksiyon grubu */
      GSimpleActionGroup *action_group = g_simple_action_group_new ();
      GSimpleAction *change_category_action = g_simple_action_new ("change-category", G_VARIANT_TYPE_STRING);
      g_signal_connect (change_category_action, "activate", G_CALLBACK (on_category_changed), new_row);
      g_action_map_add_action (G_ACTION_MAP (action_group), G_ACTION (change_category_action));
      g_object_unref (change_category_action);
      gtk_widget_insert_action_group (GTK_WIDGET (new_row), "app", G_ACTION_GROUP (action_group));
      g_object_unref (action_group);

      /* Kategori menüsü */
      GMenu *category_menu = g_menu_new ();
      if (new_category != APP_CATEGORY_MALICIOUS)
        {
          GMenuItem *item = g_menu_item_new ("Zararlı", "app.change-category");
          g_menu_item_set_action_and_target_value (item, "app.change-category", g_variant_new_string ("malicious"));
          g_menu_append_item (category_menu, item);
          g_object_unref (item);
        }
      if (new_category != APP_CATEGORY_SAFE)
        {
          GMenuItem *item = g_menu_item_new ("Güvenli", "app.change-category");
          g_menu_item_set_action_and_target_value (item, "app.change-category", g_variant_new_string ("safe"));
          g_menu_append_item (category_menu, item);
          g_object_unref (item);
        }
      if (new_category != APP_CATEGORY_UNKNOWN)
        {
          GMenuItem *item = g_menu_item_new ("Bilinmeyen", "app.change-category");
          g_menu_item_set_action_and_target_value (item, "app.change-category", g_variant_new_string ("unknown"));
          g_menu_append_item (category_menu, item);
          g_object_unref (item);
        }
      gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (category_button), G_MENU_MODEL (category_menu));
      g_object_unref (category_menu);
      adw_action_row_add_prefix (new_row, category_button);

      /* Sağ tık */
      GtkGesture *right_click = gtk_gesture_click_new ();
      gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (right_click), GDK_BUTTON_SECONDARY);
      g_signal_connect_swapped (right_click, "pressed", G_CALLBACK (gtk_menu_button_popup), category_button);
      gtk_widget_add_controller (GTK_WIDGET (new_row), GTK_EVENT_CONTROLLER (right_click));

      /* Uygulama Simgesi (Yer tutucu) */
      GtkWidget *icon = gtk_image_new_from_resource ("/com/muha/AppManager/icons/AndroidHead.svg");
      gtk_image_set_pixel_size (GTK_IMAGE (icon), 32);
      adw_action_row_add_prefix (new_row, icon);

      /* Detay düğmesi */
      GtkWidget *details_button = gtk_button_new_from_icon_name ("dialog-information-symbolic");
      gtk_widget_set_valign (details_button, GTK_ALIGN_CENTER);
      gtk_widget_add_css_class (details_button, "flat");
      gtk_widget_set_tooltip_text (details_button, "Uygulama Detayları");
      g_signal_connect (details_button, "clicked", G_CALLBACK (on_details_clicked), new_row);
      adw_action_row_add_suffix (new_row, details_button);

      /* Başlık ve alt başlık */
      adw_preferences_row_set_title (ADW_PREFERENCES_ROW (new_row), app_info_get_label (app));

      gchar *subtitle = g_strdup_printf ("%s (%s)",
                                         app_info_get_package_name (app),
                                         app_info_get_app_type (app) == APP_TYPE_SYSTEM ? "Sistem" : "Kullanıcı");
      adw_action_row_set_subtitle (new_row, subtitle);
      g_free (subtitle);

      /* Aktivasyon widget'ı */
      adw_action_row_set_activatable_widget (new_row, check);

      gtk_list_box_append (target_lists[i], GTK_WIDGET (new_row));
    }

  /* Boş liste kontrolü */
  gboolean has_unknown = gtk_widget_get_first_child (GTK_WIDGET (self->unknown_list)) != NULL;
  gboolean has_malicious = gtk_widget_get_first_child (GTK_WIDGET (self->malicious_list)) != NULL;
  gboolean has_safe = gtk_widget_get_first_child (GTK_WIDGET (self->safe_list)) != NULL;

  gtk_stack_set_visible_child_name (self->unknown_stack, has_unknown ? "list" : "empty");
  gtk_stack_set_visible_child_name (self->malicious_stack, has_malicious ? "list" : "empty");
  gtk_stack_set_visible_child_name (self->safe_stack, has_safe ? "list" : "empty");

  update_selection_status (self);
}

static void
on_app_toggled (GtkCheckButton *check,
                gpointer        user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  GtkWidget *row = gtk_widget_get_ancestor (GTK_WIDGET (check), ADW_TYPE_ACTION_ROW);
  AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

  if (app)
    {
      app->is_selected = gtk_check_button_get_active (GTK_CHECK_BUTTON (check));
      sync_selection_for_app (self, app);
      update_selection_status (self);
    }
}

static void
populate_app_list (MainWindow *self, GList *packages)
{
  GList *l;

  /* Listeyi doldur */
  for (l = packages; l != NULL; l = l->next)
    {
      AppInfo *app = l->data;
      AppCategory category = app_info_get_category (app);

      /* Her kategori için ayrı row oluştur */
      GtkListBox *target_lists[4] = {
        self->all_list,
        category == APP_CATEGORY_UNKNOWN ? self->unknown_list : NULL,
        category == APP_CATEGORY_MALICIOUS ? self->malicious_list : NULL,
        category == APP_CATEGORY_SAFE ? self->safe_list : NULL
      };

      for (int i = 0; i < 4; i++)
        {
          if (target_lists[i] == NULL)
            continue;

          AdwActionRow *row = ADW_ACTION_ROW (adw_action_row_new ());

          /* AppInfo nesnesini row'a bağla */
          g_object_set_data_full (G_OBJECT (row), "app-info", g_object_ref (app), g_object_unref);

          /* 1. Checkbox (En sol) */
          GtkWidget *check = gtk_check_button_new ();

          /* Zararlı uygulamaları varsayılan olarak seç */
          if (category == APP_CATEGORY_MALICIOUS)
            {
              gtk_check_button_set_active (GTK_CHECK_BUTTON (check), TRUE);
              app->is_selected = TRUE;
            }

          gulong handler_id = g_signal_connect (check, "toggled", G_CALLBACK (on_app_toggled), self);
          g_object_set_data (G_OBJECT (check), "handler-id", (gpointer)handler_id);
          adw_action_row_add_prefix (row, check);

          /* Checkbox'ı sakla */
          g_object_set_data (G_OBJECT (row), "selection-checkbox", check);

          /* Aktivasyon widget'ı olarak ayarla */
          adw_action_row_set_activatable_widget (row, check);

          /* 2. Kategori değiştirme düğmesi */
          GtkWidget *category_button = gtk_menu_button_new ();
          gtk_widget_set_valign (category_button, GTK_ALIGN_CENTER);
          gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (category_button), "view-list-symbolic");
          gtk_widget_add_css_class (category_button, "flat");
          gtk_widget_set_tooltip_text (category_button, "Kategoriyi Değiştir");

          /* Aksiyon grubu oluştur */
          GSimpleActionGroup *action_group = g_simple_action_group_new ();
          GSimpleAction *change_category_action = g_simple_action_new ("change-category", G_VARIANT_TYPE_STRING);
          g_signal_connect (change_category_action, "activate", G_CALLBACK (on_category_changed), row);
          g_action_map_add_action (G_ACTION_MAP (action_group), G_ACTION (change_category_action));
          g_object_unref (change_category_action);
          gtk_widget_insert_action_group (GTK_WIDGET (row), "app", G_ACTION_GROUP (action_group));
          g_object_unref (action_group);

          /* Kategori menüsü oluştur */
          GMenu *category_menu = g_menu_new ();
          if (category != APP_CATEGORY_MALICIOUS)
            {
              GMenuItem *item = g_menu_item_new ("Zararlı", "app.change-category");
              g_menu_item_set_action_and_target_value (item, "app.change-category", g_variant_new_string ("malicious"));
              g_menu_append_item (category_menu, item);
              g_object_unref (item);
            }
          if (category != APP_CATEGORY_SAFE)
            {
              GMenuItem *item = g_menu_item_new ("Güvenli", "app.change-category");
              g_menu_item_set_action_and_target_value (item, "app.change-category", g_variant_new_string ("safe"));
              g_menu_append_item (category_menu, item);
              g_object_unref (item);
            }
          if (category != APP_CATEGORY_UNKNOWN)
            {
              GMenuItem *item = g_menu_item_new ("Bilinmeyen", "app.change-category");
              g_menu_item_set_action_and_target_value (item, "app.change-category", g_variant_new_string ("unknown"));
              g_menu_append_item (category_menu, item);
              g_object_unref (item);
            }
          gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (category_button), G_MENU_MODEL (category_menu));
          g_object_unref (category_menu);
          adw_action_row_add_prefix (row, category_button);

          /* Sağ tık menüsü için GestureClick ekle */
          GtkGesture *right_click = gtk_gesture_click_new ();
          gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (right_click), GDK_BUTTON_SECONDARY);
          g_signal_connect_swapped (right_click, "pressed", G_CALLBACK (gtk_menu_button_popup), category_button);
          gtk_widget_add_controller (GTK_WIDGET (row), GTK_EVENT_CONTROLLER (right_click));

          /* 3. Uygulama Simgesi (Yer tutucu) */
          GtkWidget *icon = gtk_image_new_from_resource ("/com/muha/AppManager/icons/AndroidHead.svg");
          gtk_image_set_pixel_size (GTK_IMAGE (icon), 32);
          adw_action_row_add_prefix (row, icon);

          /* Başlık ve alt başlık */
          adw_preferences_row_set_title (ADW_PREFERENCES_ROW (row), app_info_get_label (app));

          gchar *subtitle;
          const gchar *type_str = (app_info_get_app_type (app) == APP_TYPE_SYSTEM) ? "Sistem" : "Kullanıcı";
          subtitle = g_strdup_printf ("%s (%s)", app_info_get_package_name (app), type_str);
          adw_action_row_set_subtitle (row, subtitle);
          g_free (subtitle);

          /* Detay düğmesi (Suffix) */
          GtkWidget *details_button = gtk_button_new_from_icon_name ("dialog-information-symbolic");
          gtk_widget_set_valign (details_button, GTK_ALIGN_CENTER);
          gtk_widget_add_css_class (details_button, "flat");
          gtk_widget_set_tooltip_text (details_button, "Uygulama Detayları");
          g_signal_connect (details_button, "clicked", G_CALLBACK (on_details_clicked), row);
          adw_action_row_add_suffix (row, details_button);

          gtk_list_box_append (target_lists[i], GTK_WIDGET (row));
        }
    }

  /* Boş liste kontrolü - stack'leri güncelle */
  gboolean has_unknown = FALSE;
  gboolean has_malicious = FALSE;
  gboolean has_safe = FALSE;
  GtkWidget *child;

  child = gtk_widget_get_first_child (GTK_WIDGET (self->unknown_list));
  has_unknown = (child != NULL);

  child = gtk_widget_get_first_child (GTK_WIDGET (self->malicious_list));
  has_malicious = (child != NULL);

  child = gtk_widget_get_first_child (GTK_WIDGET (self->safe_list));
  has_safe = (child != NULL);

  gtk_stack_set_visible_child_name (self->unknown_stack, has_unknown ? "list" : "empty");
  gtk_stack_set_visible_child_name (self->malicious_stack, has_malicious ? "list" : "empty");
  gtk_stack_set_visible_child_name (self->safe_stack, has_safe ? "list" : "empty");

  update_selection_status (self);
}

typedef struct {
  MainWindow *self;
  gchar *serial;
  GList *user_apps; /* AppInfo listesi */
  gchar *previous_stack_child; /* Yükleme öncesi content_stack'in görünür çocuğu */
} LoadAppsData;

static void
load_apps_data_free (LoadAppsData *data)
{
  g_free (data->serial);
  g_free (data->previous_stack_child);
  /* user_apps listesindeki AppInfo'lar populate_app_list'te ref alındı veya
     burada unref edilmeli. Ancak biz listeyi birleştirip topluca işleyeceğiz. */
  if (data->user_apps)
    g_list_free_full (data->user_apps, g_object_unref);
  g_free (data);
}

static void
on_system_packages_loaded (GObject      *source_object G_GNUC_UNUSED,
                           GAsyncResult *result,
                           gpointer      user_data)
{
  LoadAppsData *data = user_data;
  MainWindow *self = data->self;
  GError *error = NULL;
  GList *pkg_names = NULL;
  GList *system_apps = NULL;
  GList *all_apps = NULL;
  GList *l;

  pkg_names = adb_get_packages_finish (result, &error);

  if (error)
    {
      g_warning ("Sistem uygulamaları alınamadı: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      /* String listesini AppInfo listesine çevir */
      for (l = pkg_names; l != NULL; l = l->next)
        {
          gchar *pkg_name = l->data;
          AppInfo *app = app_info_new (pkg_name, APP_TYPE_SYSTEM);
          system_apps = g_list_append (system_apps, app);
        }
      /* pkg_names listesini (stringler dahil) temizle */
      g_list_free_full (pkg_names, g_free);
    }

  /* Kullanıcı ve sistem uygulamalarını birleştir */
  /* Not: data->user_apps sahipliğini alıyoruz, data_free'de double-free olmamalı.
     Bu yüzden data->user_apps'i NULL yapıyoruz. */
  all_apps = g_list_concat (data->user_apps, system_apps);
  data->user_apps = NULL;

  populate_app_list (self, all_apps);

  /* all_apps listesindeki AppInfo'lar populate_app_list içinde row'lara ref ile bağlandı.
     Burada listeyi temizlerken unref yapmalıyız. */
  g_list_free_full (all_apps, g_object_unref);

  /* Yükleme bitti, listeyi göster */
  if (data->previous_stack_child)
    gtk_stack_set_visible_child_name (self->content_stack, data->previous_stack_child);
  else
    gtk_stack_set_visible_child_name (self->content_stack, "all");

  load_apps_data_free (data);
}

static void
on_user_packages_loaded (GObject      *source_object G_GNUC_UNUSED,
                         GAsyncResult *result,
                         gpointer      user_data)
{
  LoadAppsData *data = user_data;
  GError *error = NULL;
  GList *pkg_names = NULL;
  GList *l;

  pkg_names = adb_get_packages_finish (result, &error);

  if (error)
    {
      g_warning ("Kullanıcı uygulamaları alınamadı: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      /* String listesini AppInfo listesine çevir */
      for (l = pkg_names; l != NULL; l = l->next)
        {
          gchar *pkg_name = l->data;
          AppInfo *app = app_info_new (pkg_name, APP_TYPE_USER);
          data->user_apps = g_list_append (data->user_apps, app);
        }
      g_list_free_full (pkg_names, g_free);
    }

  /* Şimdi sistem uygulamalarını yükle */
  adb_get_packages_async (data->serial, "-s", NULL, on_system_packages_loaded, data);
}

static void
load_applications (MainWindow *self, const gchar *serial)
{
  /* Tüm listeleri temizle */
  GtkWidget *child;
  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list))))
    gtk_list_box_remove (self->all_list, child);
  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self->unknown_list))))
    gtk_list_box_remove (self->unknown_list, child);
  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self->malicious_list))))
    gtk_list_box_remove (self->malicious_list, child);
  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self->safe_list))))
    gtk_list_box_remove (self->safe_list, child);

  /* Yükleme başlıyor, loading ekranını göster */
  LoadAppsData *data = g_new0 (LoadAppsData, 1);
  data->self = self;
  data->serial = g_strdup (serial);

  const gchar *current_child = gtk_stack_get_visible_child_name (self->content_stack);
  if (g_strcmp0 (current_child, "loading") == 0)
    data->previous_stack_child = g_strdup ("all");
  else
    data->previous_stack_child = g_strdup (current_child);

  gtk_stack_set_visible_child_name (self->content_stack, "loading");

  /* Kullanıcı uygulamalarını yükle (-3) */
  adb_get_packages_async (serial, "-3", NULL, on_user_packages_loaded, data);
}

static void
on_device_selected (GtkDropDown *dropdown,
                    GParamSpec  *pspec G_GNUC_UNUSED,
                    gpointer     user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);

  /* Dropdown NULL ise veya geçerli bir dropdown değilse self->device_dropdown kullan */
  if (dropdown == NULL || !GTK_IS_DROP_DOWN (dropdown))
    {
      if (self->device_dropdown)
        dropdown = self->device_dropdown;
      else
        return; /* Yapacak bir şey yok */
    }

  GtkStringList *model = GTK_STRING_LIST (gtk_drop_down_get_model (dropdown));

  if (model == NULL)
    return;

  guint selected_index = gtk_drop_down_get_selected (dropdown);
  const gchar *selected_string = gtk_string_list_get_string (model, selected_index);

  if (selected_string)
    {
      gboolean is_unauthorized = g_str_has_suffix (selected_string, "(Yetkisiz)");

      /* Serial'ı bulmak için string'i parse et */
      /* Format: "Model (Serial)" veya "Model (Serial) (Yetkisiz)" */

      /* Sondan başa doğru ilk '(' karakterini bul */
      /* Eğer yetkisiz ise, sondaki "(Yetkisiz)" kısmını atla */

      gchar *temp_string = g_strdup (selected_string);
      if (is_unauthorized)
        {
          gchar *unauth_pos = g_strrstr (temp_string, " (Yetkisiz)");
          if (unauth_pos)
            *unauth_pos = '\0';
        }

      /* Şimdi format "Model (Serial)" olmalı. Son '(' ve son ')' arasını al */
      char *last_open_paren = strrchr (temp_string, '(');
      char *last_close_paren = strrchr (temp_string, ')');

      if (last_open_paren && last_close_paren && last_close_paren > last_open_paren)
        {
          /* Parantezlerin içi serial */
          *last_close_paren = '\0';
          gchar *serial = g_strdup (last_open_paren + 1);

          if (is_unauthorized)
            {
               gtk_stack_set_visible_child_name (self->main_stack, "unauthorized");
            }
          else
            {
               gtk_stack_set_visible_child_name (self->main_stack, "content");
               load_applications (self, serial);
            }
          g_free (serial);
        }
      else
        {
          /* Parse edilemedi */
          g_warning ("Cihaz stringi parse edilemedi: %s", selected_string);
          gtk_stack_set_visible_child_name (self->main_stack, "no-device");
        }

      g_free (temp_string);
    }
}

static void
on_search_toggle_action (GSimpleAction *action G_GNUC_UNUSED,
                         GVariant      *parameter G_GNUC_UNUSED,
                         gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);

  if (self->search_toggle)
    {
      gboolean active = gtk_toggle_button_get_active (self->search_toggle);
      gtk_toggle_button_set_active (self->search_toggle, !active);
    }
}

static void
on_remove_apps_action (GSimpleAction *action G_GNUC_UNUSED,
                       GVariant      *parameter G_GNUC_UNUSED,
                       gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);

  /* TODO: Implement remove apps functionality */
  /* For now, just activate the button's action if it's sensitive */
  if (self->remove_button && gtk_widget_get_sensitive (GTK_WIDGET (self->remove_button)))
    {
      gtk_widget_activate (GTK_WIDGET (self->remove_button));
    }
}

static void
on_cancel_action (GSimpleAction *action G_GNUC_UNUSED,
                  GVariant      *parameter G_GNUC_UNUSED,
                  gpointer       user_data G_GNUC_UNUSED)
{
  /* TODO: Implement cancel operation functionality */
  /* This should cancel any ongoing operation like backup or removal */
  g_debug ("Cancel action triggered");
}

static void
on_focus_device_action (GSimpleAction *action G_GNUC_UNUSED,
                        GVariant      *parameter G_GNUC_UNUSED,
                        gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);

  if (self->device_dropdown)
    {
      /* Activate the dropdown to open the popup */
      gtk_widget_activate (GTK_WIDGET (self->device_dropdown));
    }
}

static void
filter_list_box (GtkListBox *listbox, const gchar *search_text, gboolean search_app_name, gboolean search_package_id)
{
  GtkWidget *child;

  if (search_text == NULL || *search_text == '\0')
    {
      /* Arama metni boşsa tüm satırları göster */
      for (child = gtk_widget_get_first_child (GTK_WIDGET (listbox)); child != NULL; child = gtk_widget_get_next_sibling (child))
        {
          gtk_widget_set_visible (child, TRUE);
        }
      return;
    }

  gchar *search_lower = g_utf8_strdown (search_text, -1);

  for (child = gtk_widget_get_first_child (GTK_WIDGET (listbox)); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
      AppInfo *app = g_object_get_data (G_OBJECT (child), "app-info");
      gboolean visible = FALSE;

      if (app)
        {
          if (search_app_name)
            {
              gchar *label_lower = g_utf8_strdown (app_info_get_label (app), -1);
              if (strstr (label_lower, search_lower) != NULL)
                {
                  visible = TRUE;
                }
              g_free (label_lower);
            }

          if (!visible && search_package_id)
            {
              gchar *package_lower = g_utf8_strdown (app_info_get_package_name (app), -1);
              if (strstr (package_lower, search_lower) != NULL)
                {
                  visible = TRUE;
                }
              g_free (package_lower);
            }
        }
      gtk_widget_set_visible (child, visible);
    }
  g_free (search_lower);
}

static void
on_search_changed (GtkSearchEntry *entry, gpointer user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  const gchar *search_text = gtk_editable_get_text (GTK_EDITABLE (entry));
  gboolean search_app_name = gtk_toggle_button_get_active (self->filter_app_name);
  gboolean search_package_id = gtk_toggle_button_get_active (self->filter_package_id);

  filter_list_box (self->all_list, search_text, search_app_name, search_package_id);
  filter_list_box (self->unknown_list, search_text, search_app_name, search_package_id);
  filter_list_box (self->malicious_list, search_text, search_app_name, search_package_id);
  filter_list_box (self->safe_list, search_text, search_app_name, search_package_id);
}

static void on_filter_toggled (GtkToggleButton *button, gpointer user_data);

static void
on_search_toggled (GtkToggleButton *button, gpointer user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  gboolean active = gtk_toggle_button_get_active (button);

  gtk_widget_set_visible (GTK_WIDGET (self->search_entry), active);
  // TODO: Ayarlardan okunacak değer ile değiştir
  gtk_widget_set_visible (GTK_WIDGET (self->search_filters_box), active);

  if (active)
    {
      gtk_widget_grab_focus (GTK_WIDGET (self->search_entry));
    }
  else
    {
      /* Arama alanını temizle ve filtreyi sıfırla */
      gtk_editable_set_text (GTK_EDITABLE (self->search_entry), "");
    }
}

static void
on_filter_toggled (GtkToggleButton *button G_GNUC_UNUSED, gpointer user_data)
{
    MainWindow *self = MAIN_WINDOW (user_data);
    /* Sadece arama değişikliği sinyalini tetikle, mantık zaten orada. */
    g_signal_emit_by_name (self->search_entry, "search-changed", NULL);
}

static void
main_window_init (MainWindow *self)
{
  GSimpleActionGroup *actions;
  GActionEntry action_entries[] = {
    { "refresh", on_refresh_action, NULL, NULL, NULL, {0} },
    { "select-all", on_select_all_action, NULL, NULL, NULL, {0} },
    { "select-all-global", on_select_all_global_action, NULL, NULL, NULL, {0} },
    { "search-toggle", on_search_toggle_action, NULL, NULL, NULL, {0} },
    { "remove-apps", on_remove_apps_action, NULL, NULL, NULL, {0} },
    { "cancel", on_cancel_action, NULL, NULL, NULL, {0} },
    { "focus-device", on_focus_device_action, NULL, NULL, NULL, {0} },
  };

  gtk_widget_init_template (GTK_WIDGET (self));

  if (self->sort_dropdown)
    {
      gtk_widget_set_visible (GTK_WIDGET (self->sort_dropdown), FALSE);
    }

  actions = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (actions),
                                   action_entries,
                                   G_N_ELEMENTS (action_entries),
                                   self);
  gtk_widget_insert_action_group (GTK_WIDGET (self), "win", G_ACTION_GROUP (actions));

  /* Sinyal bağlantıları */
  g_signal_connect_swapped (self->content_stack, "notify::visible-child-name",
                            G_CALLBACK (update_button_labels), self);
  self->device_handler_id = g_signal_connect (self->device_dropdown, "notify::selected", G_CALLBACK (on_device_selected), self);
  g_signal_connect (self->search_toggle, "toggled", G_CALLBACK (on_search_toggled), self);
  g_signal_connect (self->search_entry, "search-changed", G_CALLBACK (on_search_changed), self);
  g_signal_connect (self->filter_app_name, "toggled", G_CALLBACK (on_filter_toggled), self);
  g_signal_connect (self->filter_package_id, "toggled", G_CALLBACK (on_filter_toggled), self);

  /* Başlangıçta cihazları kontrol et */
  refresh_devices (self);
}

GtkWindow *
main_window_new (GtkApplication *app)
{
  return g_object_new (MAIN_TYPE_WINDOW, "application", app, NULL);
}
