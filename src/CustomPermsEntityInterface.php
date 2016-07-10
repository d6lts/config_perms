<?php

namespace Drupal\config_perms;

use Drupal\Core\Config\Entity\ConfigEntityInterface;

/**
 * Provides an interface for defining Custom perms entity entities.
 */
interface CustomPermsEntityInterface extends ConfigEntityInterface {

  /**
   *
   */
  public function getStatus();

  /**
   *
   */
  public function getPath();

}
