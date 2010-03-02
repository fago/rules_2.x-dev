<?php
// $Id$

/**
 * @file Contains ui functions for rules.
 */
class RulesRuleUI extends RulesPluginContainerUI {

  public function form(&$form, &$form_state, $options = array(), $rule) {

    $form['elements']['conditions'] = array(
      '#type' => 'container',
      '#title' => t('If:'),
    );

    // Make sure the condition is intialized and then get it.
    $rule->conditions();
    RulesPluginContainerUI::form($form['elements']['conditions'], $form_state, $options, $rule->property('conditions'));

    $form['elements']['conditions']['add'] = array(
      '#markup' => t(l('+ Add condition', RULES_UI_RULE_PATH . '/' . $rule->name . '/add/condition')),
    );

    $form['elements']['actions'] = array(
      '#type' => 'container',
      '#title' => t('Do:'),
    );

    //TODO: Add separate condition/action form for adding the add link.
    parent::form($form['elements']['actions'], $form_state, $options, $rule, $rule->actions());

    $form['elements']['actions']['add'] = array(
      '#markup' => t(l('+ Add action', RULES_UI_RULE_PATH . '/' . $rule->name . '/add/action')),
    );

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
    $form_state['rules_ui_rule'] = $rule;

  }

  public function form_validate(&$form, &$form_state, $rule) {
    $rule->label = $form_state['values']['label'];
    if (isset($form_state['values']['weight']))
      $rule->weight = isset($form_state['values']['weight']);
    if (!$rule->integrityCheck()) {
      form_set_error('', t('Something went wrong, while saving the rule!'));
    }
  }

  public function form_submit(&$form, &$form_state, $rule) {
    drupal_set_message('Rule "' . $rule->label . '" saved');
    $rule->save();
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