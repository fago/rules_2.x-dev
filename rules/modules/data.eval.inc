<?php
// $Id$

/**
 * @file
 * Contains often used rules data wrappers
 *
 * @addtogroup rules
 * @{
 */

/**
 * Action: Modify data.
 */
function rules_action_data_set($data, $value, $settings, $state, $element) {
  $wrapper = $state['execution arguments']['data'];
  $wrapper = $wrapper instanceof RulesDataMetadataWrapperInterface ? $wrapper->metadataWrapper($state) : $wrapper;
  try {
    // Update the value first then save changes, if possible.
    $wrapper->set($value);
  }
  catch (EntityMetadataWrapperException $e) {
    throw new RulesException('Unable to modify data "@selector": ' . $e->getMessage(), array('@selector' => $settings['data:select']));
  }
  $info = $wrapper->info();
  // We always have to save the changes in the parent entity. E.g. when the node
  // author is changed, we don't want to save the author but the node.
  RulesDataWrapper::metadataWrapperSaveChanges($info['parent'], $state);
}

/**
 * Action: Fetch data
 */
function rules_action_data_fetch($type, $id, $revision) {
  $info = entity_get_info($type);

  // Support the revision parameter, if applicable.
  if (isset($info['object keys']['revision'])) {
    $conditions = array($info['object keys']['revision'] => $revision);
  }
  $entity = reset(entity_load($type, array($id), isset($conditions) ? $conditions : array()));
  if (!$entity) {
    throw new RulesException('Unable to load @entity with id "@id"', array('@id' => $id, '@entity' => $type));
  }
  return array('data_fetched' => $entity);
}

/**
 * Action: Create data
 */
function rules_action_data_create($type) {
  $args = func_get_args();
  // Settings is always the forth last parameter that is passed.
  $settings = $args[count($args)-4];
  $values = array();
  foreach (array_keys($settings['#info']['parameter']) as $i => $name) {
    // Remove the parameter name prefix 'param_'.
    $values[substr($name, 6)] = $args[$i+1];
  }
  $data = entity_metadata_entity_create($type, $values)->value();
  return array('data_created' => $data);
}

/**
 * Action: Save data
 */
function rules_action_data_save($data, $immediate = FALSE, $settings, $state, $element) {
  // Usually rules cares for saving, however it won't happen immediately.
  if ($immediate && $wrapper = $state['execution arguments']['data']) {
    $wrapper = $wrapper instanceof RulesDataMetadataWrapperInterface ? $wrapper->metadataWrapper($state) : $wrapper;
    RulesDataWrapper::metadataWrapperSaveChanges($wrapper, $state, $immediate);
    // We have already saved, so don't do it again.
    return FALSE;
  }
}

/**
 * Action: Delete data
 */
function rules_action_data_delete($data, $settings, $state, $element) {
  $wrapper = $state['execution arguments']['data'];
  $wrapper = $wrapper instanceof RulesDataMetadataWrapperInterface ? $wrapper->metadataWrapper($state) : $wrapper;
  if ($var = $state['wrapper map']->getVariable($wrapper)) {
    $var->delete();
  }
  else {
    $wrapper->delete();
  }
}

/**
 * Action: Add a list item
 */
function rules_action_data_list_add($list, $item, $pos = 'end', $settings) {
  switch ($pos) {
    case 'start':
      array_unshift($list, $item);
      break;

    default:
      $list[] = $item;
      break;
  }
  return array('list' => $list);
}

/**
 * Action: Remove a list item
 */
function rules_action_data_list_remove($list, $item) {
  foreach (array_keys($list, $item) as $key) {
    unset($list[$key]);
  }
  return array('list' => $list);
}

/**
 * Condition: Compare data
 */
function rules_condition_data_is($data, $op, $value) {
  switch ($op) {
    default:
    case '=':
      return $data == $value;
    case '<=':
      return $data <= $value;
    case '>=':
      return $data >= $value;
    case 'IN':
      return is_array($value) && in_array($data, $value);
  }
}


class RulesTextWrapper extends RulesDataWrapper {
  // TODO: input form
  // TODO: use input evaluator in input form

  protected $itemName = 'text';
}

class RulesDecimalWrapper extends RulesTextWrapper {
  // TODO: input form
  // TODO: use input evaluator in input form (calculation)

  protected $itemName = 'decimal';
}

class RulesIntegerWrapper extends RulesDecimalWrapper {
  // TODO: input form
  // TODO: use input evaluator in input form (calculation)

  protected $itemName = 'integer';
}

class RulesDateWrapper extends RulesDataWrapper {
  // TODO: input form
  // TODO: customize selector form and add offset (duration)

  protected $itemName = 'date';
}

class RulesDurationWrapper extends RulesDataWrapper {
  // TODO: input form
  // TODO: use input evaluator in input form (calculation)

  protected $itemName = 'duration';
}

class RulesBooleanWrapper extends RulesDataWrapper {
  // TODO: input form
  // TODO: customize selector form and add negation option

  protected $itemName = 'boolean';
}

class RulesURIWrapper extends RulesTextWrapper {
  // TODO: input form
  // TODO: use input evaluator in input form

  protected $itemName = 'uri';
}

/**
 * Wraps lists, which in the end are numerically indexed arrays.
 */
class RulesDataListWrapper extends RulesDataWrapper {

  protected $itemName = 'list';

  public static function availableTypeParameter($type = 'list') {
    $cache = rules_get_cache();
    foreach ($cache['data_info'] as $name => $info) {
      $return[$name] = t('list of @item-type items', array('@item-type' => $info['label']));
    }
    return isset($return) ? $return : array();
  }

  /**
   * Override to pass a parametrized type as the metadata wrapper can handle it.
   *
   * @return EntityMetadataWrapper
   */
  public function metadataWrapper(array $state = NULL) {
    if (!isset($this->wrapper)) {
      $data = &$this->get($state);
      $type = $this->parameter ? 'list' . '<' . $this->parameter . '>' : $this->type;
      $this->wrapper = entity_metadata_wrapper($type, $data);
    }
    return $this->wrapper;
  }
}

/**
 * Defines a generic wrapper for all core entities.
 */
class RulesDataEntityWrapper extends RulesDataWrapper {

  /**
   * Returns the available parameter for entities, which are the known bundles.
   */
  public static function availableTypeParameter($type) {
    if (($info = entity_get_info($type)) && !empty($info['bundles'])) {
      foreach ($info['bundles'] as $name => $bundle_info) {
        $return[$name] = $bundle_info['label'];
      }
    }
    return isset($return) ? $return : array();
  }

  protected $itemName = 'entity';

  /**
   * Implements RulesDataMetadataWrapperInterface.
   */
  public function metadataWrapper(array $state = NULL) {
    if (!isset($this->wrapper)) {
      $data = &$this->get($state);
      // We treat the type parameter as bundle.
      $this->wrapper = entity_metadata_wrapper($this->type, $data, $this->parameter ? array('bundle' => $this->parameter) : array());
      // Register this as the variable for the wrapper in the state's map.
      if (!empty($state['wrapper map'])) {
        $state['wrapper map']->add($this->wrapper, $this);
      }
    }
    return $this->wrapper;
  }
}

