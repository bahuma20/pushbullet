<?php
/**
 * @file
 * Class for pushbullet in PHP.
 */

class PushbulletPushBulletException extends Exception {
  // Exception thrown by PushBullet.
}

class PushbulletPushBullet {
  /**
   * Constructs a pushbullet object.
   */
  public function __construct($secret) {
    // Check if cURL is loaded.
    if (!function_exists('curl_init')) {
      throw new PushbulletPushBulletException('cURL library is not loaded.');
    }

    $this->_api_key = $secret;

    // Get all devices associated with the API key.
    // This is also a more reliable way of API key validation.
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, self::API_HOST . '/devices');
    curl_setopt($curl, CURLOPT_USERPWD, $this->_api_key);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
    $response = curl_exec($curl);

    if ($response === FALSE) {
      throw new PushbulletPushBulletException('cURL Error: ' . curl_error($curl));
    }

    $http_code = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    curl_close($curl);

    if ($http_code != 200) {
      throw new PushbulletPushBulletException('Unable to authenticate. HTTP Error '
      . $this->pushBulletErrors[$http_code]);
    }

    /* Check PHP version to determine whether a JSON
    big int workaround should be used. */
    if (version_compare(PHP_VERSION, '5.4.0', '>=')) {
      $this->_all_devices = json_decode($response, TRUE, 512, JSON_BIGINT_AS_STRING);
    }
    else {
      $this->_all_devices = json_decode(preg_replace('/\"([a-zA-Z0-9_]+)\":\s?(\d{14,})/',
      '"${1}":"${2}"', $response), TRUE);
    }

    if ($this->_all_devices) {
      $this->_my_devices = $this->_all_devices['devices'];
      $this->_shared_devices = $this->_all_devices['shared_devices'];
    }
    else {
      throw new PushbulletPushBulletException('Unable to decode JSON response.');
    }
  }

  /**
   * Gets devices.
   */
  public function getDevices() {
    return $this->_all_devices;
  }

  /**
   * Gets my devices.
   */
  public function getMyDevices() {
    return $this->_my_devices;
  }

  /**
   * Gets shared devices.
   */
  public function getSharedDevices() {
    return $this->_shared_devices;
  }

  /**
   * Pushes a note.
   */
  public function pushNote($devices, $title, $body) {
    return $this->push($devices, 'note', $title, $body);
  }

  /**
   * Pushes an address.
   */
  public function pushAddress($devices, $name, $address) {
    return $this->push($devices, 'address', $name, $address);
  }

  /**
   * Pushes a list.
   */
  public function pushList($devices, $title, $items) {
    return $this->push($devices, 'list', $title, $items);
  }

  /**
   * Pushes a file.
   */
  public function pushFile($devices, $file_name) {
    return $this->push($devices, 'file', $file_name, NULL);
  }

  /**
   * Pushes a link.
   */
  public function pushLink($devices, $title, $url) {
    return $this->push($devices, 'link', $title, $url);
  }


  const API_HOST = 'https://api.pushbullet.com/api';
  protected $apiKey;
  protected $allDevices;
  protected $my_devices;
  protected $shared_devices;

  protected $pushBulletErrors = array(
    400 => '400 Bad Request. Missing a required parameter.',
    401 => '401 Unauthorized. No valid API key provided.',
    402 => '402 Request Failed.',
    403 => '403 Forbidden. The API key is not valid for that request.',
    404 => '404 Not Found. The requested item doesn\'t exist.',
    500 => '500 Internal Server Error.',
    502 => '502 Bad Gateway.',
    503 => '503 Service Unavailable.',
    504 => '504 Gateway Timeout.',
  );

  /**
   * Builds the curl query.
   */
  protected function buildCurlQuery($device_id, $type, $primary, $secondary) {
    switch ($type) {
      case 'note':
        if (empty($primary) && !empty($secondary)) {
          // PushBullet doesn't set a placeholder title if it's not supplied.
          $primary = 'Note';
        }
        elseif (empty($primary) && empty($secondary)) {
          throw new PushbulletPushBulletException('Note: No title and body supplied.');
        }

        $query_data = http_build_query(array(
          'device_iden' => $device_id,
          'type' => 'note',
          'title' => $primary,
          'body' => $secondary,
        ));
        break;

      case 'address':
        if (!$secondary) {
          throw new PushbulletPushBulletException('Address: No address supplied.');
        }

        $query_data = http_build_query(array(
          'device_iden' => $device_id,
          'type' => 'address',
          'name' => $primary,
          'address' => $secondary,
        ));
        break;

      case 'list':
        if (empty($primary) && !empty($secondary)) {
          // PushBullet doesn't set a placeholder title if it's not supplied.
          $primary = 'List';
        }
        elseif (empty($secondary)) {
          /* PushBullet accepts absolutely empty to-do lists,
		  but there's no point.*/
          throw new PushbulletPushBulletException('List: No items supplied.');
        }

        $query_data = http_build_query(array(
          'device_iden' => $device_id,
          'type' => 'list',
          'title' => $primary,
          'items' => $secondary,
        ));

        // Remove array keys in square brackets.
        $query_data = preg_replace('/%5B[0-9]+%5D/i', '', $query_data);
        break;

      case 'file':
        $full_file_path = realpath($primary);

        if (!is_readable($full_file_path)) {
          throw new PushbulletPushBulletException('File: File does not exist or is unreadable.');
        }

        if (filesize($full_file_path) > 25 * 1024 * 1024) {
          throw new PushbulletPushBulletException('File: File size exceeds 25 MB.');
        }

        $query_data = array(
          'device_iden' => $device_id,
          'type' => 'file',
          'file' => '@' . $full_file_path . ';filename=' . basename($full_file_path),
        );
        break;

      case 'link':
        if (empty($secondary)) {
          throw new PushbulletPushBulletException('Link: No URL supplied.');
        }
        $query_data = http_build_query(array(
          'device_iden' => $device_id,
          'type' => 'link',
          'title' => $primary,
          'url' => $secondary,
        ));
    }

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, self::API_HOST . '/pushes');
    curl_setopt($curl, CURLOPT_USERPWD, $this->_api_key);
    curl_setopt($curl, CURLOPT_POST, TRUE);
    curl_setopt($curl, CURLOPT_POSTFIELDS, $query_data);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
    $response = curl_exec($curl);

    if ($response === FALSE) {
      throw new PushbulletPushBulletException('cURL Error: ' . curl_error($curl));
    }

    $http_code = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    curl_close($curl);

    if ($http_code >= 400) {
      throw new PushbulletPushBulletException('Push failed for device '
      . $device_id
      . '. HTTP Error '
      . $this->pushBulletErrors[$http_code]);
    }

    return TRUE;
  }

  /**
   * Does the push.
   */
  protected function push($push_to, $push_type, $primary, $secondary) {
    if (empty($this->_all_devices)) {
      throw new PushbulletPushBulletException('Push: No devices found.');
    }

    if (is_string($push_to) && $push_to != 'all' && $push_to != 'my' && $push_to != 'shared') {
      return $this->buildCurlQuery($push_to, $push_type, $primary, $secondary);
    }
    elseif (is_array($push_to)) {
      // Push to multiple devices in an array.
      // Check if the device ID is in the list of devices we have permissions to push to.
      $failed_devices = array();
      foreach ($push_to as $device) {
        if ($this->inArray($device, $this->_all_devices)) {
          $this->buildCurlQuery($device, $push_type, $primary, $secondary);
        }
        else {
          $failed_devices[] = $device;
        }
      }

      if (!empty($failed_devices)) {
        throw new PushbulletPushBulletException('Push failed for devices: ' . implode(', ', $failed_devices));
      }
    }
    elseif ($push_to == 'all' || $push_to == 'my' || $push_to == 'shared') {
      // Push to my devices, if any.
      if (($push_to == 'all' || $push_to == 'my') && !empty($this->_my_devices)) {
        foreach ($this->_my_devices as $my_device) {
          $this->buildCurlQuery($my_device['iden'], $push_type, $primary, $secondary);
        }
      }
      elseif ($push_to == 'my' && empty($this->_my_devices)) {
        throw new PushbulletPushBulletException('Push: No own devices found.');
      }

      // Push to shared devices, if any.
      if (($push_to == 'all' || $push_to == 'shared') && !empty($this->_shared_devices)) {
        foreach ($this->_shared_devices as $shared_device) {
          $this->buildCurlQuery($shared_device['iden'], $push_type, $primary, $secondary);
        }
      }
      elseif ($push_to == 'shared' && empty($this->_shared_devices)) {
        throw new PushbulletPushBulletException('Push: No shared devices found.');
      }
    }
    else {
      throw new PushbulletPushBulletException('Push: Invalid device definition (' . $push_to . ').');
    }

    return TRUE;
  }

  /**
   * Multidimensional in_array().
   */
  protected function inArray($needle, $haystack, $strict = FALSE) {
    if (is_array($haystack)) {
      foreach ($haystack as $item) {
        if (($strict ? $item === $needle : $item == $needle) || (is_array($item) &&
          $this->inArray($needle, $item, $strict))) {
          return TRUE;
        }
      }
    }

    return FALSE;
  }
}
