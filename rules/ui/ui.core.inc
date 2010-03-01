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
   *    - 'show elements': Whether to include the 'elements' fieldset for
   *      editing the actual configuration. Defaults to TRUE.
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
   * //TODO: allow modules to specify them in the plugin_info hook.
   */
  public function operations();

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
class RulesAbstractPluginDefaults extends FacesExtender {

  public function implementsFaces() {
    return array('RulesPluginImplInterface');
  }

  public function execute($element) {
    throw new RulesException($element->getPluginName() .": Execution implementation is missing.");
  }

  public function validate($element) {}
  public function process($element) {}
  public function form($element) {}
  public function form_validate($element) {}
  public function form_submit($element) {}

  public function access($element) {
    $info = $element->property('info');
    if (!empty($info['access callback']) && !call_user_func($info['access callback'], $element->property('itemName'), $element->getElementName())) {
      return FALSE;
    }
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

  public function operations($element) {}
  public function buildContent($element) {}
  public function help($element) {}
}


/**
 * Faces UI extender for all kind of Rules plugins. Provides various useful
 * methods for any rules UI.
 */
class RulesPluginUI extends FacesExtender {

  public function implementsFaces() {
    return array('RulesPluginUIInterface');
  }

  public function form(&$form, &$form_state, $options = array(), $element) {
    $form = array();
    $form['#theme'] = 'rules_plugins';
    $form['head'] = array(
      //TODO: Remove child element id.
      '#markup' => check_plain($element->label()) . '-' . self::childElementId($element),
    );

    $form['operator'] = $element->operations();
    $form['settings'] = $element->buildContent();
  }

  public function buildContent($element) {
    $form['#markup'] = t('Parameter:');
    foreach($element->parameterInfo() as $name => $parameter) {
      $form[$name] = array(
        '#markup' => $name,
      );
    }
    return $form;
  }

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
        while ($parent = $element->property('parent')) {
          $element = $parent;
        }
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



class RulesContainerUI extends RulesPluginUI {


  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_UI_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/container'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_UI_RULE_PATH . '/' . $rule . '/delete/' . $element_id . '/container'),
    );

    $form['add_or'] = array(
      '#markup' => l(t('+ Add or'), RULES_UI_RULE_PATH . '/' . $rule . '/add/or'),
    );
    $form['add_and'] = array(
      '#markup' => l(t('+ Add and'), RULES_UI_RULE_PATH . '/' . $rule . '/add/and'),
    );
    $form['add_loop'] = array(
      '#markup' => l(t('+ Add Loop'), RULES_UI_RULE_PATH . '/' . $rule . '/delete/loop'),
    );
    return $form;
  }
}
