<?php
/**
 * @file
 * Functions for the push form
 */

/**
 * Page callback: Pushbullet settings.
 *
 * @see pushbullet_menu()
 */
function pushbullet_push_form($form, &$form_state) {
  $form['pushbullet_push_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
  );
  $form['pushbullet_push_message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#required' => TRUE,
  );
  $form['submit_button'] = array(
    '#type' => 'submit',
    '#value' => t('Push to my devices'),
  );
  return $form;
}

/**
 * Pushing messages.
 */
function pushbullet_push_form_submit($form, &$form_state) {
  try {
    $p = new PushbulletPushBullet(variable_get('pushbullet_api_key'));
    $p->pushNote('my', $form_state['values']['pushbullet_push_title'], $form_state['values']['pushbullet_push_message']);
    drupal_set_message(t('Your message was pushed'), 'status');
  }
  catch (PushbulletPushBulletException $e) {
    drupal_set_message(t("<b>An error occurred:</b> @message", array('@message' => $e->getMessage())), 'error');
  }
}
