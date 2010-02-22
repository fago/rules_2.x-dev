<?php
// $Id$

/**
 * @file Contains ui functions for rules.
 */
class RulesReactionRuleUI extends FacesExtender {

  public function implementsFaces() {
    return array('RulesPluginUiInterface');
  }

  public function form(&$form, &$form_state, $options = array(), $rule) {
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

    $form['elements'] = array(
      '#type' => 'fieldset',
      '#title' => t('Rule elements'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $form['elements']['events'] = array(
      '#type' => 'fieldset',
      '#title' => t('On'),
    );

    $events = rules_fetch_data('event_info');
    foreach($rule->events() as $event_name) {
      $form['elements']['events'][$event_name] = array(
        '#markup' => $events[$event_name]['label'],
        '#prefix' => '<div>',
        '#suffix' => '&nbsp;' . l(t('delete'), RULES_ADMIN_RULE_PATH) . '</div>',
      );
    }

    $form['elements']['events']['add'] = array(
      '#markup' => t(l('+ Add event', RULES_ADMIN_RULE_PATH . '/add')),
    );

    $form['elements']['conditions'] = array(
      '#type' => 'fieldset',
      '#title' => t('If:'),
    );

    foreach($rule->conditions() as $id => $condition) {
      $condition->form($form['elements']['conditions']['elements'], $form_state);
    }

    $form['elements']['conditions']['add'] = array(
      '#markup' => t(l('+ Add condition', RULES_ADMIN_RULE_PATH . '/add')),
    );


    $form['elements']['actions'] = array(
      '#type' => 'fieldset',
      '#title' => t('Do:'),
    );

    foreach ($rule->actions() as $id => $action) {
      $action->form($form['elements']['actions']['elements'][$id], $form_state);
    }

    $form['elements']['actions']['add'] = array(
      '#markup' => t(l('+ Add action', RULES_ADMIN_RULE_PATH . '/add')),
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save Changes'),
    );
    $form_state['rules_ui_rule'] = &$this;

    return $form;
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

  public function buildContent($rule) {}
  public function help($rule) {}
  public function access($rule) {}
  public function operations($rule) {}
}