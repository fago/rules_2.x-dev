<?php
// $Id$

/**
 * @file Rules Admin UI
 *   Implements rule management and configuration screens.
 */

/**
 * Reaction rules overview
 */
function rules_admin_reaction_overview($form, &$form_state) {
  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
    '#collapsible' => TRUE,
    '#collapsed' => isset($form_state['values']),
  );
  $form['filter']['event'] = array(
    '#type' => 'select',
    '#title' => t('Filter by event'),
    '#options' => array(0 => '<All>') + RulesPluginUI::getOptions('event_info'),
    '#default_value' => isset($form_state['values']['event']) ? $form_state['values']['event'] : 0,
  );
  $form['filter']['category'] = array(
    '#type' => 'select',
    '#title' => t('Filter by category'),
    '#options' => array(0 => '<All>'),
    '#default_value' => isset($form_state['values']['category']) ? $form_state['values']['category'] : 0,
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Filter'));

  $event = !empty($form_state['values']['event']) ? $form_state['values']['event'] : FALSE;
  $category = !empty($form_state['values']['category']) ? $form_state['values']['category'] : FALSE;

  $form['active'] = array(
    '#type' => 'fieldset',
    '#title' => t('Active rules'),
  );
  $conditions = array('event' => $event, 'plugin' => 'reaction rule', 'active' => TRUE);
  $form['active']['table'] = RulesPluginUI::overviewTable(array_filter($conditions), t('There are no matching active reaction rules.'));


  $form['inactive'] = array(
    '#type' => 'fieldset',
    '#title' => t('Inactive rules'),
  );
  $conditions = array_filter(array('event' => $event, 'plugin' => 'reaction rule')) + array('active' => FALSE);
  $form['inactive']['table'] = RulesPluginUI::overviewTable($conditions, t('There are no matching inactive reaction rules.'));

  $form_state['#submit'] = TRUE;

  return $form;
}

/**
 * Submit callback for the reaction rules overview form.
 */
function rules_admin_reaction_overview_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Rules overview
 */
function rules_admin_components_overview($form, &$form_state) {
  drupal_set_message("Rules admin set overview");
  return $form;
}

/**
 * Define the rule settings
 */
function rules_admin_settings($form, &$form_state) {
  drupal_set_message("Rules admin settings");
  return $form;
}

/**
 * Edit a reaction rule
 */
function rules_admin_form_edit_rule($form, &$form_state, $rule = NULL) {
  $rule->form($form, $form_state);
  $form_state['rules'][$rule->name] = &$rule;
  $form['#validate'][] = 'rules_admin_form_edit_rule_validate';
  $form['#submit'][] = 'rules_admin_form_edit_rule_submit';
  return $form;
}

function rules_admin_form_edit_rule_validate($form, &$form_state) {
  foreach ($form_state['rules'] as $rule) {
    $rule->form_validate($form, $form_state);
  }
}

function rules_admin_form_edit_rule_submit($form, &$form_state) {
  foreach ($form_state['rules'] as $rule) {
    $rule->form_submit($form, $form_state);
  }
}

/**
 * Add Elements
 */
function rules_admin_add_element($form, &$form_state, $rule, $type) {
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
  $form_state['redirect'] = RULES_ADMIN_RULE_PATH .'/'. $rule->name;
  return $form;
}

function rules_admin_edit_element($form, &$form_state, $rule, $element) {

  return $form;
}
