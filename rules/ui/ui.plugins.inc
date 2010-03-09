<?php
// $Id$

/**
 * @file Contains UI for diverse plugins provided by Rules.
 */

/**
 * Rule specific UI.
 */
class RulesRuleUI extends RulesContainerPluginUI {

  protected $rule, $conditions;

  public function __construct(FacesExtendable $object) {
    parent::__construct($object);
    $this->rule = $object;
    // Make sure the condition is intialized and then get it.
    $this->rule->conditions();
    $this->conditions = $this->property('conditions');
  }

  public function form(&$form, &$form_state, $options = array()) {
    $form += array('conditions' => array(), 'actions' => array());
    $this->conditions->form($form['conditions'], $form_state, $options);

    // Add actions form.
    parent::form($form['actions'], $form_state, $options, $this->rule->actions());

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
      '#default_value' => $this->rule->label,
      '#required' => TRUE,
    );
    // TODO: Description as field
    $form['settings']['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#default_value' => $this->rule->weight,
      '#delta' => 10,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save Changes'),
    );
    $form_state['rules_config'][$this->rule->name] = $this->rule;
  }

  /**
   * Applies the values of the form to the rule configuration.
   */
  function form_extract_values($form, $form_state) {
    $this->rule->label = $form_state['values']['label'];
    $this->rule->weight = $form_state['values']['weight'];
    // Run condition and action container value extraction.
    $this->conditions->extender('RulesConditionContainerUI')->form_extract_values($form['conditions'], $form_state);
    parent::form_extract_values($form['actions'], $form_state);
  }
}

/**
 * Reaction rule specific UI.
 */
class RulesReactionRuleUI extends RulesRuleUI {

  public function form(&$form, &$form_state, $options = array()) {

    $form['elements']['events'] = array(
      '#type' => 'container',
      '#title' => t('On'),
    );

    $event_info = rules_fetch_data('event_info');
    foreach($this->rule->events() as $event_name) {
      $form['elements']['events'][$event_name] = array(
        '#markup' => check_plain($event_info[$event_name]['label']),
        '#prefix' => '<div>',
        '#suffix' => '&nbsp;<span class="rules_rule_event">' . l(t('delete'), RULES_UI_RULE_PATH) . '</span></div>',
      );
    }

    $form['elements']['events']['add'] = array(
      '#markup' => t(l('+ Add event', RULES_UI_RULE_PATH . '/' . $this->rule->name . '/add/event')),
    );

    parent::form($form, $form_state, $options);
  }
}