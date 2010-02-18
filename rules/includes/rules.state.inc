<?php
// $Id$

/**
 * @file Contains the rules state class.
 */

/**
 * The rules evaluation state.
 */
class RulesState extends FacesRoot {

  // The known variables.
  public $variables = array();

  // Holds info about the variables.
  protected $info = array();

  // Holds wrappers for the current arguments when executing a plugin.
  public $executionArguments;
  // Maps metadata wrappers to variables, needed to save changes.
  public $wrapperMap;

  // Keep the ids of blocked rules due to recursion prevention.
  protected $blocked = array();

  // Remember all variables existing so we can save them.
  protected $allVariables = array();

  protected $executedElement;

  /**
   * In case of recursive invocations this saves the previous state of the
   * executed configuration.
   */
  protected $oldState = NULL;


  public function __construct(RulesPlugin $element) {
    $this->executedElement = $element;
    $this->oldState = $element->property('state');
    $this->wrapperMap = new RulesWrapperMap();
    $this->blocked = isset($this->oldState) ? $this->oldState->blocked : array();
  }

  /**
   * Adds the given variable to the given execution state.
   */
  public function addVariable($name, &$data, $info) {
    if (($variable = rules_wrap_data($data, $info))) {
      $variable->setVariableName($name);
    }
    else {
      $variable = $data;
    }
    $this->info[$name] = $info + array(
      'skip save' => FALSE,
      'save' => FALSE,
      'type' => 'unknown',
      'handler' => FALSE,
    );
    $this->variables[$name] = $variable;
    $this->allVariables[] = $variable;
  }

  /**
   * Runs post-evaluation tasks, such as saving variables.
   *
   * @return RulesState
   *   In case of recursive evaluations, the previous evaluation state.
   */
  public function cleanUp(RulesPlugin $element) {
    // Make changes permanent.
    $vars = array_merge($this->wrapperMap->variables(), $this->allVariables);
    foreach ($vars as $var) {
      $var->saveChanges(NULL, $this);
    }
    unset($this->executionArguments);
    $state =& $element->property('state');
    $state = $this->oldState;
  }

  /**
   * Block a rules configuration from execution.
   */
  public function block($rules_config) {
    if (empty($rules_config->recursion) && $rules_config->id) {
      $this->blocked[$rules_config->id] = TRUE;
    }
  }

  /**
   * Unblock a rules configuration from execution.
   */
  public function unblock($rules_config) {
    if (empty($rules_config->recursion) && $rules_config->id) {
      unset($this->blocked[$rules_config->id]);
    }
  }

  /**
   * Returns whether a rules configuration should be blocked from execution.
   */
  public function isBlocked($rule_config) {
    return !empty($rule_config->id) && isset($this->blocked[$rule_config->id]);
  }

  /**
   * When cloning the evaluation state we want to have separate variables, so
   * that added variables won't appear in the original state. However we need
   * some arrays to be unique over all clones, so reference them properly.
   */
  public function __clone() {
    $state = $this->executedElement->property('state');
    $this->allVariables =& $state->allVariables;
    $this->blocked =& $state->blocked;
  }

  /**
   * Get the info about the state variables.
   */
  public function info() {
    return $this->info;
  }

  /**
   * Gets a variable.
   *
   * If necessary, the specified handler is invoked to fetch the variable.
   *
   * @param $name
   *   The name of the variable to return.
   * @return
   *   The variable or a EntityMetadataWrapper containing the variable.
   */
  public function &get($name) {
    if (!isset($this->variables[$name]) && $this->info[$name]['handler'] && function_exists($this->info[$name]['handler'])) {
      $this->variables[$name] = call_user_func_array($this->info[$name]['handler'], $this->variables);
      $this->info[$name]['handler'] = FALSE;
    }
    return $this->variables[$name];
  }

  /**
   * Updates a variable.
   *
   * @param $name
   *   The name of the variable to update.
   * @param $value
   *   The data with which to update or its EntityMetadataWrapper.
   * @param $permanent
   *   If TRUE, save the data later on. Only applicable for savable data types.
   */
  public function update($name, $value, $permanent = FALSE) {
    $this->variables[$name] = $data;
    return $permanent ? $this->saveChanges($name, FALSE) : $this;
  }

  /**
   * Apply permanent changes provided variable's data type is savable.
   *
   * @param $name
   *   The name of the variable to update.
   * @param $immediate
   *   Pass FALSE to postpone saving to later on. Then invoke it later with
   *   set to NULL, so the data is saved only if it has been previously marked
   *   to save it later. If TRUE is passed, it's immediately saved.
   */
  public function saveChanges($name, $immediate = NULL) {
    $info = &$this->info[$name];
    // Remember to save later on.
    if ($immediate === FALSE && !$info['skip save']) {
      $info['save'] = TRUE;
      if (isset($this->blocked)) {
        $info += array('blocked' => array());
        $info['blocked'] += $this->blocked;
      }
    }
    // Save now!
    elseif (($immediate || $info['save']) && !$info['skip save'] && $this->isSavable($name)) {
      rules_log('Saved variable %name of type %type.', array('%name' => $name, '%type' => $info['type']));

      if (isset($this->blocked) && !empty($info['blocked'])) {
        // Add the set of blocked elements for the recursion prevention.
        $previously_blocked = $this->blocked;
        $this->blocked += $info['blocked'];
      }
      // Actually save!
      $this->variables[$name]->save();

      if (isset($this->blocked) && isset($previously_blocked)) {
        // Restore the state's set of blocked elements.
        $this->blocked = $previously_blocked;
      }
      $info['save'] = FALSE;
      unset($info['blocked']);
    }
    return $this;
  }

  /**
   * Returns whether the variable with the given name is an savable.
   */
  public function isSavable($name) {
    $entity_info = entity_get_info();
    return isset($this->info[$name]['type']) && isset($entity_info[$this->info[$name]['type']]['save callback']);
  }

  /**
   * Returns whether the variable with the given name is an entity.
   */
  public function isEntity($name) {
    $entity_info = entity_get_info();
    return isset($this->info[$name]['type']) && isset($entity_info[$this->info[$name]['type']]);
  }
}



/**
 * A small class mapping entity metadata wrappers to variables. We need this
 * map for being able to apply intelligent saving to this data.
 */
class RulesWrapperMap {

  protected $map = array();

  public function add(EntityMetadataWrapper $wrapper, RulesDataWrapper $var) {
    if ($wrapper instanceof EntityWrapper && ($id = $wrapper->getIdentifier())) {
      // Usually we map by the data's id, as when the wrapper's data has been
      // changed (e.g. node:author) then we need a new variable.
      $id = $wrapper->type() . ':' . $id;
      $this->map[$id] = $var;
    }
    elseif ($var->isSavable()) {
      // Data being savable but having no ID can only be in a wrapper, if
      // directly derived from a variable. In that case we create a new data
      // item, which will be always associatd with that variable.
      $this->map[spl_object_hash($wrapper)] = $var;
    }
  }

  /**
   * Gets the variable for a wrapper.
   */
  public function getVariable(EntityMetadataWrapper $wrapper) {
    if (isset($this->map[spl_object_hash($wrapper)])) {
      return $this->map[spl_object_hash($wrapper)];
    }
    elseif (($id = $wrapper->getIdentifier()) && ($id = $wrapper->type() . ':' . $id) && isset($this->map[$id])) {
      return $this->map[$id];
    }
    return FALSE;
  }

  /**
   * Get all variables contained in the map.
   */
  public function variables() {
    return $this->map;
  }
}
