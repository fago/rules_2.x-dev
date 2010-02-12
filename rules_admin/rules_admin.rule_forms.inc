<?php
// $Id$

/**
 * @file Rules Admin UI
 *   Implements rule management and configuration screens.
 */

require_once('rules_admin.inc');

/**
 * Reaction rules overview
 */
function rules_admin_reaction_overview($form, &$form_state) {
	$form["filter"] = array(
		'#type' => 'fieldset',
		'#title' => t('Filter'),
    '#collapsible' => TRUE,
    '#collapsed' => isset($form_state['values']),// || count(rules_get_configured_items('rules')) < 20,
	);
  $form['filter']['events'] = array(
    '#type' => 'select',
    '#title' => t('Filter by event'),
    '#options' => array(0 => '<All>') + rules_admin_get_grouped_data('event_info'),
    '#default_value' => isset($form_state['values']['set']) ? $form_state['values']['set'] : 0,
  );
  $form['filter']['category'] = array(
    '#type' => 'select',
    '#title' => t('Filter by category'),
    '#options' => array(0 => '<All>'),
    '#default_value' => isset($form_state['values']['category']) ? $form_state['values']['category'] : 0,
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Filter'));
  
  $set = !empty($form_state['values']['set']) ? 'event_'. $form_state['values']['set'] : FALSE;
  $category = !empty($form_state['values']['category']) ? $form_state['values']['category'] : FALSE;
  
    
  return $form;
}

/**
 * Rules overview
 */
function rules_admin_sets_overview(&$form_state) {
	drupal_set_message("Rules admin set overview");
  $form = array();
  return $form;
}

/**
 * Define the rule settings
 */
function rules_admin_settings(&$form_state) {
	drupal_set_message("Rules admin settings");
  $form = array();
  return $form;
}

