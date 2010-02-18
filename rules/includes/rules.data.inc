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
class RulesDataWrapper extends RulesExtendable implements Serializable {

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

  /**
   * Makes the changes to a metadata wrapper permanent by retrieving the proper
   * rules data wrapper and using it to save changes.
   *
   * @param $wrapper
   *   The wrapper that has been set, thus its parent will be saved.
   */
  public static function metadataWrapperSaveChanges($wrapper, RulesState $state, $immediate = FALSE) {
    $info = $wrapper->info() + array('type' => $wrapper->type());
    if ($var = $state->wrapperMap->getVariable($wrapper)) {
      $var->update($wrapper)->saveChanges($immediate, $state);
    }
    // This data wrapper has no variable associated yet, so create one.
    elseif (($var = rules_wrap_data($wrapper->value(), $info)) && $var->isIdentifiable()) {
      if ($var->isSavable()) {
        $var->saveChanges($immediate, $state);
        $state->wrapperMap->add($wrapper, $var);
      }
      // Else the data is identifiable, but not savable. Nothing we can do.
    }
    elseif (isset($info['parent'])) {
      // Data was not identifiable, so try saving the parent.
      self::metadataWrapperSaveChanges($info['parent'], $state, $immediate);
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
   * @return
   *   Returns TRUE if the data is already loaded, FALSE else.
   */
  public function isLoaded() {
    return $this->info['loaded'];
  }

  /**
   * This variable is registered in the execution state with the given name.
   *
   * @param $name The name of the variable that holds this data.
   */
  public function setVariableName($name) {
    $this->name = $name;
  }

  /**
   * Returns whether this data is savable
   */
  public function isSavable() {
    return TRUE;
  }

  /**
   * Returns whether this data is identifiable
   */
  public function isIdentifiable() {
    $info = entity_get_info();
    return isset($info[$this->type]);
  }

  /**
   * Returns whether this data makes use of an input form for creating an instance on the fly.
   */
  public function hasDirectInputForm() {
    return $this->facesAs('RulesDataDirectInputFormInterface');
  }

  /**
   * Gets the information about this data type.
   */
  public function info() {
    return $this->info;
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
    if (!$this->info['loaded'] && $this->info['handler'] && function_exists($this->info['handler'])) {
      $args = isset($state) ? rules_unwrap_data($state->variables, $state, FALSE) : array();
      $this->data = call_user_func_array($this->info['handler'], $args);
      $this->info['loaded'] = TRUE;
    }
    elseif (!$this->info['loaded'] && isset($this->data) && $this->isIdentifiable()) {
      $return = reset(entity_load($this->type, array($this->data)));
      // First unset the data, so we prevent changing a referenced variable.
      unset($this->data);
      $this->data = $return;
      $this->info['loaded'] = TRUE;
      if (isset($this->name)) {
        rules_log('Loaded variable %name of type %type.', array('%name' => $this->name, '%type' => $this->type));
      }
    }
    return $this->data;
  }

  /**
   * Updates the data.
   *
   * @param $data
   *   The data with which to update or its EntityMetadataWrapper.
   * @param $permanent
   *   If TRUE, save the data later on. Only applicable for savable data types.
   */
  public function update($data, $permanent = FALSE) {
    debug('deprecated '. __FUNCTION__);
    $this->data = $data instanceof EntityMetadataWrapper ? $data->value() : $data;
    if (isset($this->wrapper) && $this->wrapper != $data) {
      // Sync with the metadata wrapper, so that selectors get the fresh data.
      $this->wrapper->set($data);
    }
    return $permanent ? $this->saveChanges(FALSE) : $this;
  }

  /**
   * Apply permanent changes provided this data type is savable.
   *
   * @param $immediate
   *   Pass FALSE to postpone saving to alter on. Then invoke it later with
   *   set to NULL, so the data is saved only if it has been previously marked
   *   to save it later. If TRUE is passed, it's immediately saved.
   */
  public function saveChanges($immediate = NULL, $state = NULL) {
    if (isset($state) && isset($this->name)) {
      return $state->saveChanges($this->name, $immediate);
    }
    // Remember to save later on.
    if ($immediate === FALSE && !$this->info['skip save']) {
      $this->info['save'] = TRUE;
      if (isset($state->blocked)) {
        $this->blocked += $state->blocked;
      }
    }
    // Save now!
    elseif (($immediate || $this->info['save']) && !$this->info['skip save'] && $this->isSavable()) {
      if (isset($this->name)) {
        rules_log('Saved variable %name of type %type.', array('%name' => $this->name, '%type' => $this->type));
      }
      if (isset($state->blocked) && !empty($this->blocked)) {
        // Add the set of blocked elements for the recursion prevention.
        $previously_blocked = $state->blocked;
        $state->blocked += $this->blocked;
      }
      // Actually save!
      $this->save();

      if (isset($state->blocked) && isset($previously_blocked)) {
        // Restore the state's set of blocked elements.
        $state->blocked = $previously_blocked;
      }
      $this->info['save'] = FALSE;
    }
    return $this;
  }

  /**
   * Implementation of the Serializable Interface.
   * We do not use __sleep() so we avoid changing the current instance.
   */
  public function serialize() {
    $data = $this->data;
    $info = $this->info;
    if ($this->isIdentifiable()) {
      // If this data type is identifiable, only serialize the id.
      $data = $this->getIdentifier();
      $info['loaded'] = FALSE;
      $info['handler'] = FALSE;
    }
    return serialize(array($data, $info, $this->skipSetUp, $this->itemName, $this->parameter));
  }

  /**
   * Implementation of the Serializable Interface.
   */
  public function unserialize($serialized) {
    list($this->data, $this->info, $this->skipSetUp, $this->itemName, $this->parameter) = unserialize($serialized);
    $this->type = $this->itemName;
    parent::__wakeup();
  }

  public function save() {
    if ($w = $this->metadataWrapper()) {
      return $w->save();
    }
  }

  public function delete() {
    if ($w = $this->metadataWrapper()) {
      return $w->delete();
    }
  }

  function metadataWrapper() {
    return NULL;
  }
}
