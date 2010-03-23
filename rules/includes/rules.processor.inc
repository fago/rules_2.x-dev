<?php
// $Id$

/**
 * @file Contains classes for data processing.
 *
 * Data processors can be used to process element arguments on evaluation time,
 * e.g. to apply input evaluators or to apply simple calculations to number
 * arguments.
 */

/**
 * Common base class for Rules data processors.
 */
abstract class RulesDataProcessor {

  /**
   * Gets the processed value.
   *
   * @param $info
   *   Info about the parameter for which we get the argument value for.
   * @param $state RulesState
   *   The rules evaluation state.
   * @param $element RulesPlugin
   *   The element for which we get the argument for.
   * @return
   *   The processed value.
   */
  abstract public function value($info, RulesState $state, $element);
}

/**
 * A processor for applying Rules input evaluators.
 */
class RulesDataInputEvalProcessor extends RulesDataProcessor {

  protected $value, $usedVars = array();

  /**
   * Returns the prepared value to store, which is either the unprocessed value
   * if nothing to do or a RulesDataProcessor object so processing is invoked on
   * evaluation time.
   *
   * @param $value
   *   The value to be processed.
   * @param $param_info
   *   The info about the parameter to prepare the value for.
   * @param $var_info
   *   An array of info about the available variables.
   */
  public static function prepareValue(&$value, $param_info, $var_info = array()) {
    if (self::evaluators($param_info)) {
      $processor = new RulesDataInputEvalProcessor($value, $param_info, $var_info = array());
      if ($processor->usedVars) {
        // An input evaluator has been used, so the value needs processing.
        $value = $processor;
      }
    }
  }

  protected function __construct($value, $param_info, $var_info = array()) {
    $this->value = $value;
    $this->prepare($param_info, $var_info);
  }

  protected function prepare($param_info, $var_info) {
    foreach (self::evaluators($param_info) as $base => $info) {
      $data = call_user_func(array($base, 'prepare'), $this->value, $var_info);
      if (isset($data)) {
        $this->usedVars[$base] = $data;
      }
    }
  }

  public function unprocessedValue() {
    return $this->value;
  }

  public function __toString() {
    return $this->value;
  }

  public function value($info, RulesState $state, $element) {
    $value = $this->value;
    $cache = rules_get_cache();
    $options = array_filter(array(
      'language' => isset($element->settings['language']) ? $element->settings['language'] : NULL,
      'callback' => isset($cache['data info'][$info['type']]['cleaning callback']) ? $cache['data info'][$info['type']]['cleaning callback'] : FALSE,
      'sanitize' => !empty($info['sanitize']),
    ));

    foreach ($this->usedVars as $base => $data) {
      $value = call_user_func(array($base, 'value'), $value, $data, $options, $state);
    }
    return $value;
  }

  /**
   * @return
   *   A renderable array of help.
   */
  public static function help(&$form, $param_info, $var_info) {
    foreach (self::evaluators($param_info) as $base => $info) {
      $form['help'][$base] = call_user_func(array($base, 'help'), $var_info);
      $form['help'][$base]['#weight'] = $info['weight'];
    }
  }


  /**
   * Returns all input evaluators that can be applied to the parameters needed
   * type.
   */
  public static function evaluators($param_info) {
    static $evaluators = array();

    if (!isset($evaluators['all'])) {
      $evaluators['all'] = rules_fetch_data('evaluator_info');
      uasort($evaluators['all'], array(__CLASS__, '_evaluator_sort'));
    }
    // Input mode isn't supported for multiple types.
    if (is_array($param_info['type'])) {
      return array();
    }
    // Filter the evaluators by type.
    if (!isset($evaluators[$param_info['type']])) {
      $evaluators[$param_info['type']] = array();
      foreach ($evaluators['all'] as $name => $info) {
        // Check whether the parameter type matches the supported types.
        if (RulesData::typesMatch($param_info, $info)) {
          $evaluators[$param_info['type']][$name] = $info;
        }
      }
    }
    return $evaluators[$param_info['type']];
  }

  public static function _evaluator_sort($a, $b) {
    return $a['weight'] < $b['weight'] ? -1 : ($a['weight'] > $b['weight'] ? 1 : 0);
  }
}

/**
 * A common interface for rules input evaluators.
 */
interface RulesDataInputEvalProcessorInterface {

  /**
   * Prepares the evalution, e.g. to determine whether the input evaluator has
   * been used.
   *
   * @param $text
   *   The text to evaluate later on.
   * @param $variables
   *   An array of info about available variables.
   * @return
   *     Arbitrary data, which is passed to the evaluator on evaluation.
   *     If NULL is returned the input evaluator will be skipped later.
   */
  public static function prepare($text, $variables);

  /**
   * Apply the input evaluator.
   *
   * @param $text
   *   The text to evaluate.
   * @param $data
   *   The data as returned from the preparation step.
   * @param $options
   *   A keyed array of settings and flags to control the processing.
   *   Supported options are:
   *   - language: A language object to be used when processing.
   *   - callback: A callback function that will be used to post-process
   *     replacements that might be incorporated, so they can be cleaned in a
   *     certain way.
   *   - sanitize: A boolean flag indicating whether incorporated replacements
   *     should be sanitized.
   * @param RulesState
   *   The rules evaluation state.
   * @return
   *   The evaluated text.
   */
  public static function value($text, $data, $options, RulesState $state);

  /**
   * Provide some usage help for the evaluator.
   *
   * @param $variables
   *   An array of info about available variables.
   * @return
   *   A renderable array.
   */
  public static function help($variables);
}


