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
  pushbullet_push("note", array(
    'title' => $form_state['values']['pushbullet_push_title'],
    'message' => $form_state['values']['pushbullet_push_message'],
    )
  );
}
