<?php
// $Id$

/**
 * @file Rules UI forms
 */


/**
 * Edit a rules configuration.
 */
function rules_ui_form_edit_rules_config($form, &$form_state, $rule) {
  $rule->form($form, $form_state);
  $form['#validate'][] = 'rules_ui_form_edit_rules_config_validate';
  $form['#submit'][] = 'rules_ui_form_edit_rules_config_submit';
  return $form;
}

/**
 * Form validation callback.
 */
function rules_ui_form_edit_rules_config_validate($form, &$form_state) {
  foreach ($form_state['rules_config'] as $config) {
    $config->form_validate($form, $form_state);
  }
}

/**
 * Form submit callback.
 */
function rules_ui_form_edit_rules_config_submit($form, &$form_state) {
  foreach ($form_state['rules_config'] as $config) {
    $config->form_submit($form, $form_state);
  }
  $form_state['rebuild'] = TRUE;
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
  $form['#tree'] = TRUE;
  $form['title'] = array(
    '#markup' => $element->getElementName(),
    '#prefix' => '<h3>',
    '#suffix' => '</h3>',
  );

  $form['arguments'] = array(
    '#type' => 'fieldset',
    '#title' => t('Arguments'),
    '#prefix' => '<div id="rules-wrapper">',
    '#suffix' => '</div>'
  );
  foreach($element->parameterInfo() as $name => $parameter) {
    $form['arguments'][$name] = array(
      '#type' => 'fieldset',
      '#title' => $parameter['label'],
      '#description' => (isset($parameter['description']) ? $parameter['description'] : ''),
    );
    $form['arguments'][$name]['switch'] = array(
      '#type' => 'select',
      '#title' => t('Select a mode'),
      '#options' => array(
       0 => 'data',
       1 => 'input',
      ),
      '#ajax' => array(
        'callback' => 'rules_ui_parameter_replace',
        'wrapper' => 'rules-wrapper',
      ),
    );
    if (empty($form_state['values']['arguments'][$name]['switch'])) {
      $form['arguments'][$name]['input'] = array(
        '#type' => 'select',
        '#title' => t('Data'),
        '#options' => array(),
      );
    }
    else{
      $form['arguments'][$name]['input'] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
      );
    }
    $form_state['rules']['paramters'][$name] = $parameter;
  }
  return $form;
}

function rules_ui_edit_element_submit($form, &$form_state) {
  //$name = $form_state['rules']['parameter']
  $form['arguments'][] = '';
}

function rules_ui_parameter_replace($form, $form_state) {

  return $form['arguments'];
}
