<?php
/**
 * @file
 * Functions for the admin form.
 */

/**
 * Page callback: Pushbullet settings.
 *
 * @see pushbullet_menu()
 */
function pushbullet_config_form($form, &$form_state) {
  $form['pushbullet_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API-Key'),
    '#default_value' => variable_get('pushbullet_api_key'),
    '#description' =>
    t('Get your API-Key at <a href="@url" target="_blank">http://pushbullet.com/account</a>',
    array('@url' => url('http://pushbullet.com/account'))),
    '#required' => TRUE,
  );
  return system_settings_form($form);
}
