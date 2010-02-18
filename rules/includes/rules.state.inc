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

  // Holds wrappers for the current arguments when executing a plugin.
  public $executionArguments;
  // Maps metadata wrappers to variables, needed to save changes.
  public $wrapperMap;

  // Keep the ids of blocked rules due to recursion prevention.
  public $blocked = array();

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
    if ($variable = rules_wrap_data($data, $info)) {
      $variable->setVariableName($name);
      $this->allVariables[] = $variable;
    }
    else {
      // If there is no wrapper class, just pass through the data.
      $variable = $data;
    }
    $this->variables[$name] = $variable;
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
