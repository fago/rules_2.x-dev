<?php
// $Id$

/**
 * @file Contains basic rules data wrappers.
 */


/**
 * Interface for any data that can be specified by an input form.
 */
interface RulesDataInputFormInterface {

  public function useEvalInput();

  /**
   * Gets an input form for creating an instance of your data type.
   */
  public function getInputForm($variable_info, $value, &$form_state);

  /**
   * Process or check the value of your data type. E.g. the number data type
   * uses this to make sure the value is a number.
   */
  public function processValue($variable_info, $value);
}




/**
 * A generic data wrapper.
 * Modules can provide further wrappers by inheriting or extending this class.
 *
 * Interfaces that may be used to extend a wrapper:
 *
 * @see RulesDataSavableInterface
 * @see RulesDataIdentifiableInterface
 * @see RulesDataDirectInputFormInterface
 */
class RulesDataWrapper extends RulesExtendable {

  /**
   * Returns whether the type match. They match if type1 is compatible to type2.
   *
   * @param $var_info
   *   The name of the type to check for whether it is compatible to type2.
   * @param $param_info
   *   The type expression to check for.
   * @return
   *   Whether the types match.
   */
  public static function typesMatch($var_info, $param_info) {
    $var_type = $var_info['type'];
    $param_type = $param_info['type'];

    if ($param_type == '*' || $param_type == 'unknown') {
      return TRUE;
    }

    if ($var_type == $param_type) {
      // Make sure the bundle matches, if specified by the parameter.
      return !isset($param_info['bundles']) || isset($var_info['bundle']) && in_array($var_info['bundle'], $param_info['bundles']);
    }

    // Parameters may specify multiple types using an array.
    $valid_types = is_array($param_type) ? $param_type : array($param_type);
    if (in_array($var_type, $valid_types)) {
      return TRUE;
    }

    // Check for sub-type relationships.
    if (!isset($param_info['bundles'])) {
      $cache = &rules_get_cache();
      self::typeCalcAncestors($cache, $var_type);

      // If one of the types is an ancestor return TRUE.
      return (bool)array_intersect_key($cache['data_info'][$var_type]['ancestors'], array_flip($valid_types));
    }
    return FALSE;
  }

  protected static function typeCalcAncestors(&$cache, $type) {
    if (!isset($cache['data_info'][$type]['ancestors'])) {
      $cache['data_info'][$type]['ancestors'] = array();
      if (isset($cache['data_info'][$type]['parent']) && $parent = $cache['data_info'][$type]['parent']) {
        $cache['data_info'][$type]['ancestors'][$parent] = TRUE;
        self::typeCalcAncestors($cache, $parent);
        // Add all parent ancestors to our own ancestors.
        $cache['data_info'][$type]['ancestors'] += $cache['data_info'][$parent]['ancestors'];
      }
      // For special lists like list<node> add in "list" as valid parent.
      if (entity_metadata_list_extract_type($type)) {
        $cache['data_info'][$type]['ancestors']['list'] = TRUE;
      }
    }
  }

  protected $hook = 'data_info';

  protected $data = NULL, $wrapper = NULL;
  protected $name, $type = 'unknown', $parameter;
  protected $info = array();

  /**
   * The rules to block during saving due to recursion prevention. Passed
   * through to the state during save(), thus any triggered events block those.
   */
  protected $blocked = array();

  /**
   * Constructor
   *
   * @param $data
   *   If available, the actual data, else NULL.
   * @param $info
   *   An array of info about this data.
   */
  function __construct(&$data = NULL, array $info = array(), $cache = array()) {
    $this->data = &$data;
    $this->info = $info + array(
      'skip save' => FALSE,
      'save' => FALSE,
      'type' => 'unknown',
      'handler' => FALSE,
      'loaded' => empty($info['handler']),
    );
    $this->type = $this->info['type'];
    $this->itemName = $this->type;
    $this->setUp();
  }


  /**
   * Returns whether this data makes use of an input form for creating an instance on the fly.
   */
  public function hasDirectInputForm() {
    return $this->facesAs('RulesDataDirectInputFormInterface');
  }

  /**
   * Gets the actual data. Be sure to keep the reference intact.
   *
   * If this data wrapper has been configured using a direct input form (via
   * the RulesDataDirectInputFormInterface) any input evaluators are applied and
   * the value is processed.
   *
   * @param $state
   *   The current execution state. Optional, but needed for applying input
   *   evaluators.
   * @throws RulesException
   *   Throws a RulesException, if the data should be loaded, but loading fails.
   * @return The data.
   */
  public function &get(RulesState $state = NULL) {
    // TODO: If there is an input value first of apply input eval + process it
    // to get the data ==> Support input evaluation.

  }

}
