<?php
// $Id$

/**
 * @file Rules UI forms
 */

/**
 * Edit a rules configuration.
 */
function rules_ui_form_edit_rules_config($form, &$form_state, $rules_config) {
  $form_state += array('rules_config' => $rules_config);
  // Add the rule configuration's form.
  $rules_config->form($form, $form_state, array('show settings' => TRUE, 'button' => TRUE));
  return $form;
}

/**
 * Edit a rules configuration form validation callback.
 */
function rules_ui_form_edit_rules_config_validate($form, &$form_state) {
  $form_state['rules_config']->form_validate($form, $form_state);
}

/**
 * Edit a rules configuration form submit callback.
 */
function rules_ui_form_edit_rules_config_submit($form, &$form_state) {
  $form_state['rules_config']->form_submit($form, $form_state);
  // Rebuild instead of redirect as this is faster.
  $form_state['rebuild'] = TRUE;
  // We need to clear the values so the new weight-id combinations can take
  // affect on rebuild.
  $form_state['input'] = array();
}

/**
 * Form for deleting rule config.
 */
function rules_ui_form_delete_rules_config($form, &$form_state, $rules_config) {
  $form_state += array('rules_config' => $rules_config);
  $confirm_question = t('Are you sure you want to delete the %plugin %label', array('%plugin' => $rules_config->plugin(), '%label' => $rules_config->label()));
  return confirm_form($form, $confirm_question, RULES_UI_PATH, t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Rule config deletion form submit callback.
 */
function rules_ui_form_delete_rules_config_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $rules_config = $form_state['rules_config'];
    $rules_config->delete();
    watchdog('content', 'Deleted rules configuration %plugin %label.', array('%plugin' => $rules_config->plugin(), '%label' => $rules_config->label()));
    drupal_set_message(t('Deleted rules configuration %plugin %label.', array('%plugin' => $rules_config->plugin(), '%label' => $rules_config->label())));
  }
}

/**
 * Add Elements
 */
function rules_ui_add_element($form, &$form_state, $rules_config, $type, $element_id) {
  if(!isset($form_state['rules_mode'])) {
    $form['element_name'] = array(
      '#type' => 'select',
      '#title' => t('Select an Event'),
      '#options' => array(RulesPluginUI::getOptions($type . '_info')),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    $form_state['rules_mode'] = 'parameter';
    $form_state['rebuild'] = TRUE;
  }
  else {
    $parent_element = RulesPluginUI::getChildElementById($rules_config, $element_id);
    switch($type) {
      case 'condition':
        $element = rules_condition($form_state['values']['element_name']);
        $parent_element->condition($element);
        break;
      case 'or':
        $element = rules_or($form_state['values']['element_name']);
        $parent_element->condition($element);
        break;
      case 'and':
        $element = rules_and($form_state['values']['element_name']);
        $parent_element->condition($element);
        break;
      case 'action':
        $element = rules_action($form_state['values']['element_name']);
        $parent_element->action($element);
        break;
      case 'loop':
        $element = rules_loop($form_state['values']['element_name']);
        $parent_element->action($element);
        break;
    }

    $element->form($form, $form_state, array('button' => TRUE));
    $form_state['redirect'] = RulesPluginUI::path($rules_config->name, '');

    $form['submit']['#validate'][] = 'rules_ui_edit_element_validate';
    $form['submit']['#submit'][] = 'rules_ui_edit_element_submit';

    $form_state += array('rules_element' => $parent_element);
  }

  return $form;
}

/**
 * Configure a rule element.
 */
function rules_ui_edit_element($form, &$form_state, $rules_config, $element) {
  $form_state += array('rules_element' => $element);
  $form_state['rules_element']->form($form, $form_state, array('button' => TRUE));
  return $form;
}

/**
 * Validate the element configuration.
 */
function rules_ui_edit_element_validate($form, &$form_state) {
  $form_state['rules_element']->form_validate($form, $form_state);
}

/**
 * Submit the element configuration.
 */
function rules_ui_edit_element_submit($form, &$form_state) {
  $form_state['rules_element']->form_submit($form, $form_state);
  $config_name = $form_state['rules_element']->root()->name;
  $form_state['redirect'] = RULES_UI_RULE_PATH . '/' . $config_name;
}

/**
 * Submit handler for switching the parameter input mode.
 */
function rules_ui_parameter_replace_submit($form, &$form_state) {
  if (isset($form_state['triggering_element'])) {
    $name = $form_state['triggering_element']['#parameter'];
    $form_state['parameter_mode'][$name] = $form_state['parameter_mode'][$name] == 'selector' ? 'input' : 'selector';
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback replacing the parameter form.
 */
function rules_ui_parameter_replace($form, $form_state) {
  return $form['parameter'];
}

/**
 * Add a new event.
 */
function rules_ui_add_event($form, &$form_state, $rules_config) {
  $form_state += array('rules_config' => $rules_config);
  $form['event'] = array(
    '#type' => 'select',
    '#title' => t('Select an Event'),
    '#options' => RulesPluginUI::getOptions('event_info'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form_state['redirect'] = RulesPluginUI::path($rules_config->name);
  return $form;
}

/**
 * Validate the event configuration.
 */
function rules_ui_add_event_validate($form, &$form_state) {
  if (!empty($form_state['values']['event'])) {
    $rules_config = $form_state['rules_config'];
    $rules_config->event($form_state['values']['event']);
    try {
      $rules_config->integrityCheck();
    }
    catch (RulesException $e) {
      //TODO: Add visual feedback which element had troubles.
      form_set_error('', $e->getMessage());
    }
  }
}

/**
 * Submit the event configuration.
 */
function rules_ui_add_event_submit($form, &$form_state) {
  if (!empty($form_state['values']['event'])) {
    $rules_config = $form_state['rules_config'];
    $rules_config->save();
    drupal_set_message(t('Add event to rule %label', array('%label' => $rules_config->label())));
  }
}

/**
 * Form to remove a event from a rule.
 */
function rules_ui_remove_event($form, &$form_state, $rules_config, $event) {
  $form_state += array('rules_config' => $rules_config, 'rules_event' => $event);
  $confirm_question = t('Are you sure you want to remove %event from the %plugin %label', array('%plugin' => $rules_config->plugin(), '%label' => $rules_config->label(), '%event' => $event));
  $form_state['redirect'] = RulesPluginUI::path($rules_config->name);
  return confirm_form($form, $confirm_question, RULES_UI_RULE_PATH, t('This action cannot be undone.'), t('Remove'), t('Cancel'));
}

/**
 * Validate the event configuration.
 */
function rules_ui_remove_event_validate($form, &$form_state) {
  $rules_config = $form_state['rules_config'];
  $rules_config->removeEvent($form_state['rules_event']);
  try {
    $rules_config->integrityCheck();
  }
  catch (RulesException $e) {
    //TODO: Add visual feedback which element had troubles.
    form_set_error('', $e->getMessage());
  }
}

/**
 * Submit the event configuration.
 */
function rules_ui_remove_event_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $rules_config = $form_state['rules_config'];
    $rules_config->save();
    drupal_set_message(t('Event %event was removed from %plugin %label', array('%event' => $form_state['rules_event'], '%label' => $rules_config->label(), '%plugin' => $rules_config->plugin())));
  }
}
