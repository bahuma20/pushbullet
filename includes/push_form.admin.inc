<?php
/**
 * @file
 * Functions for the push form.
 */

/**
 * Page callback: Pushbullet settings.
 *
 * @see pushbullet_menu()
 */
function pushbullet_push_form($form, &$form_state) {
  $form['pushbullet_type'] = array(
    '#type' => 'select',
    '#title' => t('Push type'),
    '#options' => array('note' => t('Note'), 'link' => t('Link')),
    '#required' => TRUE,
  );

  $form['pushbullet_type_note'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Push a note'),
    '#states' => array(
      'visible' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'note'),
      ),
    ),
  );
  $form['pushbullet_type_note']['pushbullet_push_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#states' => array(
      'required' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'note'),
      ),
    ),
  );
  $form['pushbullet_type_note']['pushbullet_push_message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#states' => array(
      'required' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'note'),
      ),
    ),
  );

  $form['pushbullet_type_link'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Push a link'),
    '#states' => array(
      'visible' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'link'),
      ),
    ),
  );
  $form['pushbullet_type_link']['pushbullet_push_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#states' => array(
      'required' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'link'),
      ),
    ),
  );
  $form['pushbullet_type_link']['pushbullet_push_url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#states' => array(
      'required' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'link'),
      ),
    ),
  );
  $form['pushbullet_type_link']['pushbullet_push_message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#states' => array(
      'required' => array(
        ':input[name="pushbullet_type"]' => array('value' => 'link'),
      ),
    ),
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
  switch ($form_state['values']['pushbullet_type']) {
    case 'note':
      $parameters['title'] = $form_state['values']['pushbullet_type_note']['pushbullet_push_title'];
      $parameters['message'] = $form_state['values']['pushbullet_type_note']['pushbullet_push_message'];
      break;

    case 'link':
      $parameters['title'] = $form_state['values']['pushbullet_type_link']['pushbullet_push_title'];
      $parameters['url'] = $form_state['values']['pushbullet_type_link']['pushbullet_push_url'];
      $parameters['message'] = $form_state['values']['pushbullet_type_link']['pushbullet_push_message'];
  }

  pushbullet_push($form_state['values']['pushbullet_type'], $parameters);
}
