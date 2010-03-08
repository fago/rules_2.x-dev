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

/**
 * Edit an action or condition
 */
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
    $link_id = drupal_html_id('rules-parameter-switch-' . $name);
    $form['arguments'][$name]['link'] = array(
      '#type' => 'link',
      '#href' => '#',
      '#title' => t('Switch'),
      // #id is used for ajax js code generation.
      '#id' => $link_id,
      // Needed to actually output the #id.
      '#options' => array('attributes' => array('id' => $link_id)),
      '#process' => array('ajax_process_form'),
      '#prefix' => '<div class="rules_hidden rules_parameter_switch">',
      '#suffix' => '</div>',
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'rules') . '/ui/rules.ui.js'),
        'css' => array(drupal_get_path('module', 'rules') . '/ui/rules.ui.css'),
      ),
      '#ajax' => array(
        'callback' => 'rules_ui_parameter_replace',
        'wrapper' => 'rules-wrapper',
        'event' => 'click',
        // button #name => #value
        'trigger_as' => array('param_' . $name => t('Switch')),
      ),
    );
    $form['arguments'][$name]['switch'] = array(
      '#type' => 'submit',
      '#name' => 'param_' . $name,
      '#value' => t('Switch'),
      '#submit' => array('rules_ui_parameter_replace_submit'),
      '#ajax' => array(
        'callback' => 'rules_ui_parameter_replace',
        'wrapper' => 'rules-wrapper',
      ),
    );
    // Init mode.
    $form_state['param_modes'][$name] = isset($form_state['param_modes'][$name]) ? $form_state['param_modes'][$name] : 'selector';
    // Switch mode by button.
    if (isset($form_state['values']['param_' . $name])) {
      $form_state['param_modes'][$name] = $form_state['param_modes'][$name] != 'selector' ? 'selector' : 'input';
    }
    if (isset($form_state['clicked_button']) && $form_state['clicked_button']['#name'] == 'param' . $name) {
      $form_state['param_modes'][$name] = $form_state['param_modes'][$name] != 'selector' ? 'selector' : 'input';
    }
    if ($form_state['param_modes'][$name] == 'selector') {
      $form['arguments'][$name]['input'] = array(
        '#type' => 'select',
        '#title' => t('Data'),
        '#options' => array(),
      );
    }
    else {
      $form['arguments'][$name]['input'] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
      );
    }
  }
  return $form;
}

/**
 * Replacing the parameter input mode without javascript
 */
function rules_ui_parameter_replace_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $form['arguments'][] = '';
}

/**
 * Replacing the parameter input mode with javascript
 */
function rules_ui_parameter_replace($form, $form_state) {

  return $form['arguments'];
}
