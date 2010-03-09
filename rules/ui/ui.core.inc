<?php
// $Id$

/**
 * @file Contains core ui functions.
 */

/**
 * Plugin UI Interface.
 */
interface RulesPluginUIInterface {

  /**
   * Returns the whole configuration form of this rules configuration. For rule
   * elements that are part of a configuration this method just returns the
   * elements configuration form.
   *
   * @param $form
   *   The form array.
   * @param $form_state
   *   The current form state.
   * @param $options
   *   An optional array of options with the known keys:
   *    - 'show settings': Whether to include the 'settings' fieldset for
   *      editing configuration settings like the label or categories. Defaults
   *      to FALSE.
   *    - 'restrict plugins: May be used to restrict the list of rules plugins
   *      that may be added to this configuration. For that set an array of
   *      valid plugins. Note that conditions and actions are always valid, so
   *      just set an empty array for just allowing those.
   *    - 'restrict conditions': Optionally set an array of condition names to
   *      restrict the conditions that are available for adding.
   *    - 'restrict actions': Optionally set an array of action names to
   *      restrict the actions that are available to for adding.
   *    - 'restrict events': Optionally set an array of event names to restrict
   *      the events that are available for adding.
   */
  public function form(&$form, &$form_state, $options = array());

  /**
   * Validate the configuration form of this rule element.
   *
   * @param $form
   * @param $form_state
   */
  public function form_validate(&$form, &$form_state);

  /**
   * Submit the configuration form of this rule element. This makes sure to
   * put the updated configuration in the form state. For saving changes
   * permanently, just call $config->save() afterwards.
   *
   * @param $form
   * @param $form_state
   */
  public function form_submit(&$form, &$form_state);


  /**
   * Returns a structured array for rendering this element.
   */
  public function buildContent();

  /**
   * Returns the help text for editing this plugin.
   */
  public function help();

  /**
   * Checks whether the user has access to configure this element or rule
   * configuration.
   */
  public function access();

  /**
   * Returns ui operations for this element.
   */
  public function operations();

}

/**
 * Faces UI extender for all kind of Rules plugins. Provides various useful
 * methods for any rules UI.
 */
class RulesPluginUI extends FacesExtender {

  public function implementsFaces() {
    return array('RulesPluginUIInterface');
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  //TODO: Show element edit form.
  public function form(&$form, &$form_state, $options = array(), $element) {

    $form['#theme'] = 'rules_element_view';
    $form['head'] = array(
      '#markup' => check_plain($element->label()),
    );
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function form_validate() {}

  /**
   * Implements RulesPluginUIInterface.
   */
  public function form_submit() {}

  /**
   * Implements RulesPluginUIInterface.
   */
  //TODO: Return info render array for use in table.
  public function buildContent($element) {
    $form['parameter']['#markup'] = t('Parameter:');
    foreach($element->parameterInfo() as $name => $parameter) {
      $form['parameter'][$name] = array(
        '#markup' => $name,
      );
    }
    return $form;
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function access($element) {
    $info = $element->property('info');
    foreach ($info['parameter'] as $name => $info) {
      if (isset($element->settings[$name . ':select']) && $wrapper = $element->call('applyDataSelector', array($element->settings[$name . ':select']))) {
        if ($wrapper->access('view') === FALSE) {
          return FALSE;
        }
        //TODO: Check input evaluator access.
      }
    }
    return TRUE;
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function operations($element) {
    $name = $element->root()->name;
    $element_id = self::childElementId($element);
    $render = array(
      '#theme' => 'links__rules',
    );
    $render['#attributes']['class'][] = 'rules-operations';
    $render['#attributes']['class'][] = 'action-links';
    $render['#links']['edit'] = array(
      '#title' => t('Edit'),
      '#href' => RulesPluginUI::path($name, 'edit', $element_id),
    );
    $render['#links']['delete'] = array(
      '#title' => t('Delete'),
      '#href' => RulesPluginUI::path($name, 'delete', $element_id),
    );
    return $render;
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function help($element) {}


  /**
   * Returns the child element associated with the given id.
   */
  public function getChildElementById(RulesPlugin $rules_config, $id) {
    self::indexRule($rules_config);
    if (isset($rules_config->map[$id])) {
      return $rules_config->map[$id];
    }
    return FALSE;
  }

  /**
   * Returns the child element id of the given child.
   */
  public function childElementId(RulesPlugin $element) {
    self::indexRule($element);
    return $element->childElementId;
  }

  /**
   * Generates child element ids for lookup up elements.
   */
  protected function indexRule(RulesPlugin $element, &$map = array()) {
    // Generate them only once.
    if (!isset($element->childElementId)) {
      $counter = &drupal_static(__FUNCTION__, 0);

      if (!$counter) {
        // Get the rules config.
        $element = $element->root();
        $element->map = array();
        $map =& $element->map;
      }
      $counter++;
      $map[$counter] = $element;
      $element->childElementId = $counter;

      if ($element instanceof RulesContainerPlugin) {
        foreach ($element as $child) {
          self::indexRule($child, $map);
        }
      }
    }
  }

  /**
   * Generates the render array for a overview configuration table for arbitrary
   * rule configs that match the given conditions.
   *
   * @param $conditions
   *   An array of conditions as needed by rules_config_load_multiple().
   * @param $empty_text
   *   The text to display if there are no configs matching the conditions.
   * @return Array
   *   A renderable array.
   */
  public static function overviewTable($conditions = array(), $empty_text) {
    $configs = rules_config_load_multiple(FALSE, $conditions);
    $rows = array();
    $event_info = rules_fetch_data('event_info');

    foreach ($configs as $name => $config) {
      $path = RULES_UI_RULE_PATH . '/' . $name;
      $row = array();
      $row[] = l($config->label, $path);
      // Add events if the configs are assigned to events.
      $events = array();
      if ($config instanceof RulesTriggerableInterface) {
        $is_triggerable = TRUE;
        foreach ($config->events() as $event_name) {
          $events[] = $event_info[$event_name]['label'];
        }
      }
      $row[] = implode(", ", $events);
      $row[] = l(t('edit'), $path, array('query' => drupal_get_destination()));
      $row[] = l(t('clone'), $path . '/clone', array('query' => drupal_get_destination()));
      $row[] = l(t('delete'), $path . '/delete', array('query' => drupal_get_destination()));
      $rows[] = $row;
    }
    $header = array(t('Label'), array('data' => !empty($is_triggerable) ? t('Event') : ''), array('data' => t('Operations'), 'colspan' => '3'));

    $table = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => $empty_text,
    );
    return $table;
  }

  /**
   * Generates a path using the given operation for the element with the given
   * id of the configuration with the given name.
   */
  public function path($name, $op, $element_id = NULL) {
    return RULES_UI_RULE_PATH . '/' . $name . '/' . $op . (isset($element_id) ? '/' . $element_id : '');
  }

  /**
   * Returns an array of options to use with a select for the items specified
   * in the given hook.
   */
  public static function getOptions($hook) {
    $sorted_data = array();
    $data = rules_fetch_data($hook);
    foreach ($data as $name => $set) {
      $sorted_data[ucfirst($set['group'])][$name] = $set['label'];
    }
    foreach ($sorted_data as $key => $set) {
      asort($set);
      $sorted_data[$key] = $set;
    }
    return $sorted_data;
  }
}

/**
 * UI for Rules Container.
 */
class RulesContainerPluginUI extends RulesPluginUI {

  /**
   * Generates a table for editing the contained elements.
   */
  public function form(&$form, &$form_state, $options = array(), $element, $iterator = NULL) {
    $form['#type'] = 'container';
    $form['#tree'] = TRUE;
    $form['#attributes']['class'][] = 'rules-container-plugin';

    // Recurse over all element childrens or use the provided iterator.
    $iterator = new RecursiveIteratorIterator(isset($iterator) ? $iterator : $element, RecursiveIteratorIterator::SELF_FIRST);
    $root_depth = $element->depth();
    foreach ($iterator as $key => $child) {
      $id = self::childElementId($child);

      $is_container = $child instanceof RulesContainerPlugin;
      $form['elements'][$id] = array(
        '#depth' => $child->depth() - $root_depth - 1,
        '#container' => $is_container,
      );
      $label = $is_container ? check_plain($child->label()) : l($child->label(), RulesPluginUI::path($element->root()->name, 'edit', $id));
      $form['elements'][$id]['label'] = array(
        '#markup' => $label,
      );
      $form['elements'][$id]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $child->weight,
      );
      $form['elements'][$id]['parent_id'] = array(
        '#type' => 'hidden',
        '#default_value' => self::childElementId($child->property('parent')),
      );
      $form['elements'][$id]['element_id'] = array(
        '#type' => 'hidden',
        '#default_value' => $id,
      );
      $form['elements'][$id]['operations'] = $child->operations();
    }
    $form['elements']['#options'] = $options;
    $form['elements']['#theme'] = 'rules_elements';
    $form['elements'] += array('#empty' => t('None'), '#heading' => t('Elements'));
  }

  /**
   * Applies the values of the form to the given rule configuration.
   */
  function form_extract_values($form, $form_state, $element) {
    // Get the right form values.
    $values = $form_state['values'];
    foreach ($form['#parents'] as $key) {
      $values = isset($values[$key]) ? $values[$key] : NULL;
    }
    // Now apply the new hierarchy.
    if (isset($values)) {
      $root_element = $element->root();
      foreach ($values['elements'] as $id => $data) {
        $child = self::getChildElementById($root_element, $id);
        $child->weight = $data['weight'];
        $parent = self::getChildElementById($root_element, $data['parent_id']);
        $child->setParent($parent ? $parent : $element);
      }
      $element->sortChildren(TRUE);
    }
  }

  public function operations($element) {
    $ops = parent::operations($element);
    $add_ops = self::addOperations($element);
    $ops['#links'] += $add_ops['#links'];
    return $ops;
  }

  /**
   * Gets the Add-* operations for the given element.
   */
  public function addOperations($element) {
    $name = $element->root()->name;
    $render = array(
      '#theme' => 'links__rules',
    );
    $render['#attributes']['class'][] = 'rules-operations-add';
    $render['#attributes']['class'][] = 'action-links';
    foreach (rules_fetch_data('plugin_info') as $plugin => $info) {
      if (!empty($info['embeddable']) && $element instanceof $info['embeddable']) {
        $render['#links']['add_' . $plugin] = array(
          'title' => t('Add !name', array('!name' => $plugin)),
          'href' => RulesPluginUI::path($name, 'add/' . $plugin),
        );
      }
    }
    return $render;
  }
}

/**
 * UI for Rules condition container.
 */
class RulesConditionContainerUI extends RulesContainerPluginUI {

  public function form(&$form, &$form_state, $options = array(), $element, $iterator = NULL) {
    parent::form($form, $form_state, $options, $element, $iterator);
    // Add the add-* operation links.
    $form['add'] = self::addOperations($element);
    $form['#attributes']['class'][] = 'rules-condition-container';
    $form['elements']['#heading'] = t('Conditions');
  }
}

/**
 * UI for Rules action container.
 */
class RulesActionContainerUI extends RulesContainerPluginUI {

  public function form(&$form, &$form_state, $options = array(), $element, $iterator = NULL) {
    parent::form($form, $form_state, $options, $element, $iterator);
    // Add the add-* operation links.
    $form['add'] = self::addOperations($element);
    $form['#attributes']['class'][] = 'rules-action-container';
    $form['elements']['#heading'] = t('Actions');
  }
}

/**
 * UI for rules arguments
 */
class RulesArgumentUI {
  public function form_selector(&$form, &$form_state, $options = array(), $argument) {
    firep($argument);
    $form['input'] = array(
      '#type' => 'select',
      '#title' => t('Data'),
      '#options' => array(),
    );
  }

  public function form_input(&$form, &$form_state, $options = array(), $argument) {
      $form['input'] = array(
        '#type' => 'textfield',
        '#title' => t('Value'),
      );
  }
}

/**
 * Provides the interface used for implementing an abstract plugin by using
 * the Faces extension mechanism.
 */
interface RulesPluginImplInterface extends RulesPluginUIInterface {

  public function execute();

  /**
   * Validates $settings independent from a form submission.
   *
   * @throws RulesException
   *   In case of validation errors, RulesExceptions are thrown.
   */
  public function validate();

  /**
   * Processes $settings independent from a form submission. Only successfully
   * validated settings are processed, so it can be also used to prepare
   * execution dependent on the settings.
   */
  public function process();
}

/**
 * Class providing default implementations for the concrete plugin if not
 * overriden by the implementation.
 *
 * @see RulesPluginImplInterface
 */
class RulesAbstractPluginDefaults extends RulesPluginUI {

  public function implementsFaces() {
    return array('RulesPluginImplInterface');
  }

  public function execute($element) {
    throw new RulesException($element->getPluginName() .": Execution implementation is missing.");
  }

  /**
   * Implements RulesPluginImplInterface.
   */
  public function validate() {}

  /**
   * Implements RulesPluginImplInterface.
   */
  public function process() {}

  /**
   * Implements RulesPluginImplInterface.
   */
  public function access($element) {
    $info = $element->property('info');
    if (!empty($info['access callback']) && !call_user_func($info['access callback'], $element->property('itemName'), $element->getElementName())) {
      return FALSE;
    }
    return RulesPluginUI::access($element);
  }

  /**
   * Implements RulesPluginImplInterface.
   * //TODO: allow modules to specify them in the info hook.
   */
  //public function operations() {}

}

