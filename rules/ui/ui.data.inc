<?php
// $Id$

/**
 * @file Contains data type related forms.
 */


/**
 * Interface for data types providing a direct input form.
 */
interface RulesDataDirectInputForm {

  /**
   * @return Array
   *  The direct input form.
   */
  public function inputForm($name, $info, RulesPlugin $element);
}

/**
 * Default UI related class for data types.
 */
class RulesDataUI {

  /**
   * Specifies the default input mode per data type.
   */
  public function getDefaultMode() {
    return 'selector';
  }

  /**
   * Provides the selection form for a parameter.
   */
  public function selectionForm($name, $info, RulesPlugin $element) {
    $form[$name . ':select'] = array(
      '#type' => 'textfield',
      '#title' => t('Data selector'),
      '#default_value' => isset($element->settings[$name . ':select']) ? $element->settings[$name . ':select'] : '',
    );
    return $form;
  }
}

/**
 * UI for textual data.
 */
class RulesDataUIText extends RulesDataUI implements RulesDataDirectInputForm {

  public function getDefaultMode() {
    return 'input';
  }

  public function inputForm($name, $info, RulesPlugin $element) {
    $form[$name] = array(
      '#type' => 'textarea',
      '#title' => t('Value'),
      '#default_value' => isset($element->settings[$name]) ? (string)$element->settings[$name] : '',
      '#value_callback' => 'rules_data_input_text_value',
      '#rules_variables' => $element->availableVariables(),
    );
    $form['help'] = RulesDataInputEvalProcessor::help($element->availableVariables());
    return $form;
  }
}

/**
 * UI for boolean data.
 */
class RulesDataUIBoolean extends RulesDataUI implements RulesDataDirectInputForm {

  public function getDefaultMode() {
    return 'input';
  }

  public function inputForm($name, $info, RulesPlugin $element) {
    $form[$name] = array(
      '#type' => 'checkbox',
      '#title' => t('Enabled'),
      '#default_value' => !empty($element->settings[$name]),
    );
    return $form;
  }
}

