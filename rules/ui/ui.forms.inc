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
  // We need to clear the values so the new weight-id combinations can take
  // affect on rebuild.
  $form_state['input'] = array();
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

/**
 * Edit an action or condition
 */
function rules_ui_edit_element($form, &$form_state, $rule, $element) {
  drupal_set_title($element->getElementName());
  $data_info = rules_fetch_data('data_info');

  $form['parameter'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#id' => 'rules-parameter-wrapper'
  );

  foreach($element->parameterInfo() as $name => $parameter) {

    $form['parameter'][$name] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($parameter['label']),
      '#description' => check_plain(isset($parameter['description']) ? $parameter['description'] : ''),
    );
    $link_id = drupal_html_id('rules-parameter-switch-' . $name);

    $form['parameter'][$name]['link'] = array(
      '#type' => 'link',
      '#href' => '#',
      '#title' => t('Switch'),
      // #id is used for ajax js code generation.
      '#id' => $link_id,
      // Needed to actually output the #id.
      '#options' => array('attributes' => array('id' => $link_id)),
      '#process' => array('ajax_process_form'),
      // Hide the link by default and unhide it by javascript.
      '#prefix' => '<div class="rules_hidden rules_parameter_switch">',
      '#suffix' => '</div>',
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'rules') . '/ui/rules.ui.js'),
        'css' => array(drupal_get_path('module', 'rules') . '/ui/rules.ui.css'),
      ),
      '#ajax' => array(
        'callback' => 'rules_ui_parameter_replace',
        'wrapper' => 'rules-parameter-wrapper',
        'event' => 'click',
        // trigger as button #name => #value
        'trigger_as' => array('param_' . $name => t('Switch')),
      ),
    );
    $form['parameter'][$name]['switch'] = array(
      '#type' => 'submit',
      '#name' => 'param_' . $name,
      '#parameter' => $name,
      '#value' => t('Switch'),
      '#submit' => array('rules_ui_parameter_replace_submit'),
      '#ajax' => array(
        'callback' => 'rules_ui_parameter_replace',
        'wrapper' => 'rules-parameter-wrapper',
      ),
    );
    // Init the parameter mode.
    if (!isset($form_state['parameter_mode'][$name])) {
      if (isset($element->settings[$name . ':select'])) {
        $form_state['parameter_mode'][$name] = 'selector';
      }
      else {
        $form_state['parameter_mode'][$name] = 'input';
      }
    }

    if ($form_state['parameter_mode'][$name] == 'selector') {
      $form['parameter'][$name]['settings'][$name . ':select'] = array(
        '#type' => 'textfield',
        '#title' => t('Data'),
        '#default_value' => isset($element->settings[$name . ':select']) ? $element->settings[$name . ':select'] : '',
      );
    }
    else {
      $form['parameter'][$name]['settings'][$name] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
        '#default_value' => isset($element->settings[$name]) ? $element->settings[$name] : '',
      );
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['#submit'] = array('rules_ui_edit_element_submit');
  $form['#validate'] = array('rules_ui_edit_element_validate');
  $form_state['rules_element'] = &$element;
  return $form;
}

function rules_ui_edit_element_form_extract_values($form, &$form_state) {
  $element = $form_state['rules_element'];

  foreach ($form_state['values']['parameter'] as $name => $values) {
    foreach ($values['settings'] as $key => $value) {
      $element->settings[$key] = $value;
    }
  }
}

/**
 * Validate the element options
 */
function rules_ui_edit_element_validate($form, &$form_state) {
  rules_ui_edit_element_form_extract_values($form, $form_state);
  $form_state['rules_element']->form_validate($form, $form_state);
}

/**
 * Submit the element options
 */
function rules_ui_edit_element_submit($form, &$form_state) {
  // TODO: clean un-used values in settings on submit.
  $element = $form_state['rules_element'];
  rules_ui_edit_element_form_extract_values($form, $form_state);
  $element->form_submit($form, $form_state);
  $rule_name = $element->root()->name;
  $form_state['redirect'] = RULES_UI_RULE_PATH . '/' . $rule_name;
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
