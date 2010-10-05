<?php
// $Id$

/**
 * @file
 * Admin pages
 */

/**
 * Administration form
 */
function config_perms_admin_form(&$form_state, $type='') {
  $form['perms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Site Configuration Permissions'),
    '#description' => '<p>'. t("Please note that the order in which permissions are granted are as follows:") .'</p>'. 
      "<ul>" .
        "<li>" . t("User 1 still maintains full control") .
       // "<li>" . t("Any role with 'administer site configuration' will have access to all permissions listed above") .
        "<li>" . t("Nothing will be available under Site building if 'display site building menu' is not checked") .
        "<li>" . t("Nothing will be available under Site configuration if 'display site configuration menu' is not checked") .
        //"<li>" . t("Any role without 'administer site configuration' will be granted access to whatever permissions checked in the config_perms section of the permissions page.") .
      "</ul>",
      //'<p>'. t("So in other words, if you want grant roles only a few of theses items, uncheck 'administer site configuration', check one or both of the display permissions, and check whatever you want them to have access to.") .'</p>',
    '#collapsible' => 1,
    '#collapsed' => 0,
  );
  
  //config_perms_cache_rebuild();
  $perms = config_perms_perms();
  //print_r($perms);

  $form['perms']['local'] = array('#theme' => 'config_perms_form', '#tree' => TRUE, '#prefix' => '<div id="config_perms-wrapper">', '#suffix' => '</div>');
  $form['perms']['local'] += _config_perms_form($perms);
  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}

/**
 * Permissions table for the administration form 
 */
function _config_perms_form($perms) {
  $form = array();
  foreach ($perms as $key => $perm) {
    $attributes = in_array($perm->id, array(1, 2)) ? array('disabled' => $disabled) : array();
    $form[$key] = array('#tree' => TRUE);
    $form[$key]['pid'] = array('#type' => 'hidden', '#default_value' => check_plain($perm->pid));
    $form[$key]['machine_name'] = array('#type' => 'hidden', '#default_value' => check_plain($perm->machine_name));
    $form[$key]['status'] = array('#type' => 'checkbox', '#default_value' => check_plain($perm->status), '#attributes' => $attributes);
    $form[$key]['remove'] = array('#type' => 'checkbox', '#default_value' => check_plain($perm->remove), '#attributes' => $attributes) ;
    $form[$key]['name'] = array('#type' => 'textfield', '#default_value' => check_plain($perm->name), '#size' => 30, '#attributes' => $attributes);
    $form[$key]['path'] = array('#type' => 'textarea', '#default_value' => config_perms_parse_path($perm->path), '#size' => 50, '#rows' => 2, '#attributes' => $attributes);
  }
  // Add the entry fields
  $key = count($perms);
  $form[$key]['status'] = array(
    '#name' => 'status',
    '#id' => 'edit-local-status',
    '#type' => 'submit',
    '#value' => t('Add'),
    '#ahah' => array(
      'path' => 'config_perms/ahah',
      'wrapper' => 'config_perms-wrapper',
    ),
  );
  $form[$key]['name'] = array('#type' => 'textfield', '#default_value' => check_plain($new->name), '#size' => 30);
  $form[$key]['path'] = array('#type' => 'textarea', '#default_value' => check_plain($new->path), '#rows' => 2, '#size' => 50);
  $form[$key]['remove'] = array('#value' => '');
  return $form;
}

/**
 * Administration form theme function
 */
function theme_config_perms_form($form) {
  $header = array(t('Enabled'), t('Delete'), t('Name'), t('Path(s)'));
  $rows = array();
  foreach (element_children($form) as $key) {
    $row = array();
    $row[] = drupal_render($form[$key]['status']);
    $row[] = drupal_render($form[$key]['remove']);
    $row[] = drupal_render($form[$key]['name']);
    $row[] = drupal_render($form[$key]['path']);
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Interface for the AHAH management.
 */
function config_perms_ahah() {
  // Load the form from cache
  $cache = cache_get('form_' . $_POST['form_build_id'], 'cache_form');
  $form = $cache->data;
  $form_state = array('values' => $_POST);
  $tags = array();
  $check = array();
  $newest = count($_POST['local']);
  foreach ($_POST['local'] as $key => $data) {
    if (is_numeric($key)) {
      $perm = new stdClass();
      $perm->pid = check_plain($data['pid']);
      $perm->machine_name = check_plain($data['machine_name']);
      $perm->name = check_plain($data['name']);
      $perm->path = config_perms_parse_path(check_plain($data['path']));
      $perm->status = ($key == $newest-1) ? 1 : (int)isset($data['status']);
      $perm->remove = (int)isset($data['remove']);
      $perms[] = $perm;
      
      unset($cache->data['perms']['local'][$key]);
    }
  }
  
  $form = _config_perms_form($perms);
  $cache->data['perms']['local'] = array_merge($cache->data['perms']['local'], $form);
  cache_set('form_'. $_POST['form_build_id'], $cache->data, 'cache_form', $cache->expire);
  $form = $cache->data['perms'];
  $form += array(
    '#post' => $_POST,
    '#programmed' => FALSE,
    '#tree' => FALSE,
    '#parents' => array(),
  );
  drupal_alter('form', $form, array(), 'config_perms_ahah');
  $form_state['submitted'] = FALSE;
  $form = form_builder('config_perms_ahah', $form, $form_state);
  $output = theme('status_messages') . drupal_render($form['local']);
  
  print drupal_to_js(array('status' => TRUE, 'data' => $output));
  
  // Kill Devel if it's installed
  $GLOBALS['devel_shutdown'] = TRUE;
  exit;
}

/**
 * Validate handler
 */
function config_perms_admin_form_validate($form, &$form_state) {
}

/**
 * Submit handler.
 */
function config_perms_admin_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  foreach ($values['local'] as $key => $data) {
    if ($data['remove'] && !empty($data['pid'])) { // Delete
      //db_query("DELETE FROM {config_perms} WHERE id = '%s'", $data['pid']);
      config_perms_delete((object)$data);
    } 
    elseif (!empty($data['name']) && !empty($data['path'])) { // Update || Insert
      $data['path'] = config_perms_parse_path($data['path']);
      config_perms_save((object)$data);
    }
  }
  
  // Rebuild cache
  config_perms_cache_rebuild();
  
  drupal_set_message(t('The permissions have been saved.'));
}