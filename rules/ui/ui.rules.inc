<?php
// $Id$

/**
 * @file Contains ui functions for rules.
 */
class RulesRuleUI extends RulesContainerPluginUI {

  public function form(&$form, &$form_state, $options = array(), $rule) {
    // Make sure the condition is intialized and then get it.
    $rule->conditions();
    $form['conditions'] = array();
    RulesConditionContainerUI::form($form['conditions'], $form_state, $options, $rule->property('conditions'));

    $form['actions'] = array();
    RulesActionContainerUI::form($form['actions'], $form_state, $options, $rule, $rule->actions());

    // Add the settings in a separate fieldset below.
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['settings']['label'] = array(
      '#type' => 'textfield',
      '#title' => 'Label',
      '#default_value' => $rule->label,
      '#required' => TRUE,
    );
    // TODO: Description as field
    $form['settings']['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#default_value' => $rule->weight,
      '#delta' => 10,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save Changes'),
    );
    $form_state['rules_config'][$rule->name] = $rule;
  }

  /**
   * Applies the values of the form to the given rule configuration.
   */
  function form_extract_values($form, $form_state, $rule) {
    $rule->label = $form_state['values']['label'];
    $rule->weight = $form_state['values']['weight'];
    // Run condition and action container value extraction.
    RulesContainerPluginUI::form_extract_values($form['conditions'], $form_state, $rule);
    RulesActionContainerUI::form_extract_values($form['actions'], $form_state, $rule);

  }

  public function form_validate(&$form, &$form_state, $rule) {
    self::form_extract_values($form, $form_state, $rule);

    if (!$rule->access()) {
      form_set_error(NULL, t('Access violation!'));
    }
    try {
      $rule->integrityCheck();
    }
    catch (RulesException $e) {
      //TODO: Add visual feedback which element had troubles.
      form_set_error('', $e->getMessage());
    }
  }

  public function form_submit(&$form, &$form_state, $rule) {
    $rule->save();
    drupal_set_message(t('Your changes have been saved.'));
  }

}

/**
 * Reaction rule specific UI.
 */
class RulesReactionRuleUI extends RulesRuleUI {

  public function form(&$form, &$form_state, $options = array(), $rule) {

    $form['elements']['events'] = array(
      '#type' => 'container',
      '#title' => t('On'),
    );

    $event_info = rules_fetch_data('event_info');
    foreach($rule->events() as $event_name) {
      $form['elements']['events'][$event_name] = array(
        '#markup' => check_plain($event_info[$event_name]['label']),
        '#prefix' => '<div>',
        '#suffix' => '&nbsp;<span class="rules_rule_event">' . l(t('delete'), RULES_UI_RULE_PATH) . '</span></div>',
      );
    }

    $form['elements']['events']['add'] = array(
      '#markup' => t(l('+ Add event', RULES_UI_RULE_PATH . '/' . $rule->name . '/add/event')),
    );

    parent::form($form, $form_state, $options, $rule);
  }
}