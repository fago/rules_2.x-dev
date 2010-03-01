<?php
// $Id$

/**
 * @file Rules UI forms
 */


/**
 * Edit rules config
 */
function rules_ui_form_edit_rules_config($form, &$form_state, $rule = NULL) {
  $rule->form($form, $form_state);
  $form_state['rules']['rules'][$rule->name] = &$rule;
  $form['#validate'][] = 'rules_ui_form_edit_rules_config_validate';
  $form['#submit'][] = 'rules_ui_form_edit_rules_config_submit';
  return $form;
}

function rules_ui_form_edit_rules_config_validate($form, &$form_state) {
  foreach ($form_state['rules']['rules'] as $rule) {
    $rule->form_validate($form, $form_state);
  }
}

function rules_ui_form_edit_rules_config_submit($form, &$form_state) {
  foreach ($form_state['rules']['rules'] as $rule) {
    $rule->form_submit($form, $form_state);
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
  $form_state['redirect'] = RULES_ADMIN_RULE_PATH .'/'. $rule->name;
  return $form;
}

function rules_ui_edit_element($form, &$form_state, $rule, $element) {
  $form['title'] = array(
    '#markup' => $element->getElementName(),
    '#prefix' => '<h3>',
    '#suffix' => '</h3>',
  );

  $form['arguments'] = array(
    '#type' => 'fieldset',
    '#title' => t('Arguments'),
  );

  foreach($element->parameterInfo() as $name => $parameter) {
    $form['arguments'][$name] = array(
      '#type' => 'fieldset',
      '#title' => $parameter['label'],
      '#description' => (isset($parameter['description']) ? $parameter['description'] : ''),
    );
    $form['arguments'][$name]['switch'] = array(
      '#type' => 'button',
      '#value' => t('Switch to input'),
      '#ajax' => array(
        'callback' => 'rules_admin_parameter_replace',
        'wrapper' => 'rules-' . $name . '-wrapper',
        'method' => 'replace',
        'effect' => 'fade',
      ),
    );
    $form['arguments'][$name]['selector'] = array(
      '#type' => 'select',
      '#title' => t('Data'),
      '#options' => array(),
      '#prefix' => '<div id="rules-' . $name . '-wrapper" >',
      '#suffix' => '</div>',
    );
    $form_state['rules']['paramter'] = $name;
  }
  return $form;
}

function rules_ui_edit_element_submit($form, &$form_state) {
  //$name = $form_state['rules']['parameter']
  $form['arguments'][] = '';
}

function rules_ui_parameter_replace($form, $form_state) {
  $name = $form_state['rules']['paramter'];
  $form['arguments'][$name]['selector'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#prefix' => '<div id="rules-' . $name . '-wrapper" >',
  );
  return $form['arguments'][$name]['selector'];
}
