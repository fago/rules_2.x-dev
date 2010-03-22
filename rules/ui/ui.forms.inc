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
 * Form validation callback.
 */
function rules_ui_form_edit_rules_config_validate($form, &$form_state) {
  $form_state['rules_config']->form_validate($form, $form_state);
}

/**
 * Form submit callback.
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
 * Form delete rule config
 */
function rules_ui_form_delete_rules_config($form, &$form_state, $rules_config) {
  $form_state += array('rules_config' => $rules_config);
  $confirm_question = t('Are you sure you want to delete %config_name', array('%config_name' => $rules_config->name));
  return confirm_form($form,
                    $confirm_question,
                    'admin/content', t('This action cannot be undone.'),
                    t('Delete'), t('Cancel'));
}

/**
 * Form submit callback
 */
function rules_ui_form_delete_rules_config_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $rules_config = $form_state['rules_config'];
    $rules_config->delete();
    watchdog('content', 'Deleted rule %config_name .', array('%config_name' => $rules_config->name));
    drupal_set_message(t('Deleted rule %config_name ', array('%config_name' => $rules_config->name)));
  }
}

/**
 * Add Elements
 */
function rules_ui_add_element($form, &$form_state, $rule, $type) {
  if ($type == 'event') {
    $form['events'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add Event'),
    );
    $form['events']['events'] = array(
      '#type' => 'select',
      '#title' => t('Select an event'),
      '#options' => RulesPluginUI::getOptions('event_info'),
    );
    $form['submit'] = array('#type' => 'submit', '#value' => t('Add'));
  }
  $form_state['redirect'] = RULES_UI_RULE_PATH .'/'. $rule->name;
  return $form;
}

/**
 * Configure a rule element.
 */
function rules_ui_edit_element($form, &$form_state, $rule, $element) {
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
 * Configure a rule element.
 */
function rules_ui_add_event($form, &$form_state, $rule) {
  firep($rule);
}