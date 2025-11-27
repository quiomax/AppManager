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
}

static void
app_details_dialog_init (AppDetailsDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
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
      g_print ("app_details_dialog_new: CRITICAL: One or more template children are NULL!\n");
      // return self; // Yine de döndür, belki çalışır veya hatayı görürüz
    }

  gtk_label_set_text (self->app_name, app_info_get_label (app));
  gtk_label_set_text (self->package_name, app_info_get_package_name (app));

  /* Detayları ADB'den çek */
  g_debug ("app_details_dialog_new: Fetching details for %s", app_info_get_package_name (app));
  GError *error = NULL;
  if (adb_get_package_details (serial, app, &error))
    {
      g_debug ("app_details_dialog_new: Details fetched successfully");
      adw_action_row_set_subtitle (self->app_version, app_info_get_version (app) ? app_info_get_version (app) : "-");
      adw_action_row_set_subtitle (self->app_size, app_info_get_size (app) ? app_info_get_size (app) : "-");
      adw_action_row_set_subtitle (self->app_uid, app_info_get_uid (app) ? app_info_get_uid (app) : "-");
      adw_action_row_set_subtitle (self->install_date, app_info_get_install_date (app) ? app_info_get_install_date (app) : "-");
    }
  else
    {
      g_warning ("Detaylar alınamadı: %s", error->message);
      g_clear_error (&error);
    }

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

  g_debug ("app_details_dialog_new: Dialog created successfully");
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

  GtkLabel *selected_label;
  GtkLabel *malicious_label;
  GtkLabel *safe_label;
  GtkLabel *total_label;
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

  gtk_widget_class_bind_template_child (widget_class, MainWindow, selected_label);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, malicious_label);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, safe_label);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, total_label);
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
      g_debug ("on_details_clicked: Dialog created");
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
refresh_devices (MainWindow *self)
{
  GList *devices;
  GError *error = NULL;
  GtkStringList *string_list;

  /* Mevcut listeyi temizle (basitçe yeni liste oluşturarak) */
  string_list = gtk_string_list_new (NULL);

  devices = adb_get_devices (&error);

  if (error)
    {
      g_warning ("ADB hatası: %s", error->message);
      g_error_free (error);
      gtk_stack_set_visible_child_name (self->main_stack, "no-device");
      /* TODO: Hata mesajını arayüzde göster */
      gtk_drop_down_set_model (self->device_dropdown, NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (self->device_dropdown), FALSE);
      g_object_unref (string_list); /* Leak fix */
    }
  else if (devices == NULL)
    {
      gtk_stack_set_visible_child_name (self->main_stack, "no-device");
      gtk_drop_down_set_model (self->device_dropdown, NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (self->device_dropdown), FALSE);
      g_object_unref (string_list); /* Leak fix */
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
      g_list_free_full (devices, (GDestroyNotify) adb_device_free);

      if (n_devices > 0)
        {
          gtk_drop_down_set_model (self->device_dropdown, G_LIST_MODEL (string_list));
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

      /* gtk_drop_down_set_model modelin sahipliğini almaz, ancak biz string_list'i
         burada oluşturduk. GtkDropDown kendi referansını tutar.
         Bizim referansımızı serbest bırakmamız gerekir. */
      g_object_unref (string_list);
    }
}

static void
on_select_all_action (GSimpleAction *action G_GNUC_UNUSED,
                      GVariant      *parameter G_GNUC_UNUSED,
                      gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  GtkWidget *child;
  gboolean all_selected = TRUE;

  /* Önce hepsinin seçili olup olmadığını kontrol et */
  for (child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      AppInfo *app = g_object_get_data (G_OBJECT (row), "app-info");

      if (app && !app->is_selected)
        {
          all_selected = FALSE;
          break;
        }
    }

  /* Duruma göre işlem yap */
  gboolean new_state = !all_selected;

  for (child = gtk_widget_get_first_child (GTK_WIDGET (self->all_list));
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      AdwActionRow *row = ADW_ACTION_ROW (child);
      GtkWidget *prefix = adw_action_row_get_activatable_widget (row);

      if (GTK_IS_CHECK_BUTTON (prefix))
        {
          gtk_check_button_set_active (GTK_CHECK_BUTTON (prefix), new_state);
        }
    }
}

static void
on_refresh_action (GSimpleAction *action G_GNUC_UNUSED,
                   GVariant      *parameter G_GNUC_UNUSED,
                   gpointer       user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  refresh_devices (self);
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
      g_signal_connect (check, "toggled", G_CALLBACK (on_app_toggled), self);
      adw_action_row_add_prefix (new_row, check);

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
      GtkWidget *icon = gtk_image_new_from_icon_name ("application-x-executable-symbolic");
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
      update_selection_status (self);
    }
}

static void
load_applications (MainWindow *self, const gchar *serial)
{
  /* Listelere activate sinyali bağla (eğer daha önce bağlanmadıysa) */
  /* Not: Her load çağrısında tekrar bağlamamak için init'te yapmak daha doğru ama
     burada listeler temizlenip yeniden dolduruluyor, sinyal listbox'a bağlı olduğu için sorun yok.
     Ancak sinyali init fonksiyonunda bağlamak en iyisi. */
  GList *packages = NULL;
  GError *error = NULL;
  GList *l;

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

  /* 1. Kullanıcı Uygulamaları (-3) */
  GList *user_apps = adb_get_packages (serial, "-3", APP_TYPE_USER, &error);
  if (error)
    {
      g_warning ("Kullanıcı uygulamaları alınamadı: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      packages = g_list_concat (packages, user_apps);
    }

  /* 2. Sistem Uygulamaları (-s) */
  GList *system_apps = adb_get_packages (serial, "-s", APP_TYPE_SYSTEM, &error);
  if (error)
    {
      g_warning ("Sistem uygulamaları alınamadı: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      packages = g_list_concat (packages, system_apps);
    }

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

          g_signal_connect (check, "toggled", G_CALLBACK (on_app_toggled), self);
          adw_action_row_add_prefix (row, check);

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
          GtkWidget *icon = gtk_image_new_from_icon_name ("application-x-executable-symbolic");
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

  /* packages listesindeki her AppInfo için bir referans tuttuk (g_object_ref),
     bu yüzden listeyi temizlerken unref yapmalıyız */
  g_list_free_full (packages, g_object_unref);

  /* Boş liste kontrolü - stack'leri güncelle */
  gboolean has_unknown = FALSE;
  gboolean has_malicious = FALSE;
  gboolean has_safe = FALSE;

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
main_window_init (MainWindow *self)
{
  GSimpleActionGroup *actions;
  GActionEntry action_entries[] = {
    { "refresh", on_refresh_action, NULL, NULL, NULL, {0} },
    { "select-all", on_select_all_action, NULL, NULL, NULL, {0} },
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

  /* Sinyal bağlantısı */
  g_signal_connect (self->device_dropdown, "notify::selected", G_CALLBACK (on_device_selected), self);

  /* Başlangıçta cihazları kontrol et */
  refresh_devices (self);
}

GtkWindow *
main_window_new (GtkApplication *app)
{
  return g_object_new (MAIN_TYPE_WINDOW, "application", app, NULL);
}
