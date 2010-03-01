<?php
// $Id$

/**
 * @file Contains ui functions for elements like actions and conditions.
 */


/**
 * Faces UI extender for all kind of Rules plugins. Provides various useful
 * methods for any rules UI.
 */
class RulesPluginUI extends FacesExtender {

  public function implementsFaces() {
    return array('RulesPluginUI');
  }

  public function form(&$form, &$form_state, $options = array(), $element) {
    $form = array();
    $form['#theme'] = 'rules_plugins';
    $form['head'] = array(
      //TODO: Remove child element id.
      '#markup' => $element->getName() . '-' . self::childElementId($element),
    );

    $form['operator'] = $element->operations();

    $form['settings'] = array();
    $element->__call('getSettingOverview', array(&$form['settings']));

    $form['childs'] = array();
    $element->__call('getChilds', array(&$form['childs'], &$form_state));
  }

  public function getName($element){
    return $element->getElementName();
  }

  public function getSettingOverview(&$form, $element) {
    $form['#markup'] = t('Parameter:');
    foreach($element->parameterInfo() as $name => $parameter) {
      $form[$name] = array(
        '#markup' => $name,
      );
    }
  }

  public function getChilds(&$form, &$form_state, $element) {}


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
      $path = RULES_ADMIN_RULE_PATH . '/' . $name;
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

class RulesConditionUI extends RulesPluginUI {
  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/condition'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/' . $element_id  . '/condition'),
    );
    return $form;
  }
}

class RulesActionUI extends RulesPluginUI {
  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/action'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/' . $element_id  . '/action'),
    );
    return $form;
  }
}

class RulesContainerUI extends RulesPluginUI {
  public function getName() {
    return 'Loop';
  }

  public function getChilds(&$form, &$form_state, $loop) {
    foreach ($loop as $id => $children) {
      $children->form($form[$id], $form_state);
    }
  }

  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/container'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/' . $element_id . '/container'),
    );

    $form['add_or'] = array(
      '#markup' => l(t('+ Add or'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/add/or'),
    );
    $form['add_and'] = array(
      '#markup' => l(t('+ Add and'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/add/and'),
    );
    $form['add_loop'] = array(
      '#markup' => l(t('+ Add Loop'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/loop'),
    );
    return $form;
  }
}

