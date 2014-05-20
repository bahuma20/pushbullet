<?php
/**
 * @file
 * Class for pushbullet in PHP.
 */

class PushbulletPushBullet {
  protected $apiKey;

  const URL_PUSHES         = 'https://api.pushbullet.com/v2/pushes';
  const URL_DEVICES        = 'https://api.pushbullet.com/v2/devices';
  const URL_CONTACTS       = 'https://api.pushbullet.com/v2/contacts';
  const URL_UPLOAD_REQUEST = 'https://api.pushbullet.com/v2/upload-request';
  const URL_USERS          = 'https://api.pushbullet.com/v2/users';

  /**
   * Creates the PushBullet object.
   */
  public function __construct($api_key) {
    $this->apiKey = $api_key;

    if (!function_exists('curl_init')) {
      throw new PushbulletPushBulletException('cURL library is not loaded.');
    }
  }

  /**
   * Pushes a note.
   */
  public function pushNote($recipient, $title, $body = NULL) {
    return $this->push($recipient, 'note', $title, $body);
  }

  /**
   * Pushes a link.
   */
  public function pushLink($recipient, $title, $url, $body = NULL) {
    return $this->push($recipient, 'link', $title, $body);
  }

  /**
   * Pushes an address.
   */
  public function pushAddress($recipient, $name, $address) {
    return $this->push($recipient, 'address', $name, $address);
  }

  /**
   * Pushes a list.
   */
  public function pushList($recipient, $title, $items) {
    return $this->push($recipient, 'list', $title, $items);
  }

  /**
   * Pushes a file.
   */
  public function pushFile($recipient, $file_path, $mime_type = NULL) {
    return $this->push($recipient, 'file', $file_path, $mime_type);
  }

  /**
   * Gets the push history.
   */
  public function getPushHistory($modified_after = 0, $cursor = NULL) {
    $data = array();
    $data['modified_after'] = $modified_after;

    if ($cursor !== NULL) {
      $data['cursor'] = $cursor;
    }

    return $this->curlRequest(self::URL_PUSHES, 'GET', $data);
  }

  /**
   * Deletes a push.
   */
  public function deletePush($push_iden) {
    return $this->curlRequest(URL_PUSHES . '/' . $push_iden, 'DELETE');
  }

  /**
   * Gets devices.
   */
  public function getDevices() {
    return $this->curlRequest(self::URL_DEVICES, 'GET');
  }

  /**
   * Deletes a device.
   */
  public function deleteDevice($device_iden) {
    return $this->curlRequest(self::URL_DEVICES . '/' .
      $device_iden, 'DELETE');
  }

  /**
   * Gets contacts.
   */
  public function getContacts() {
    return $this->curlRequest(self::URL_CONTACTS, 'GET');
  }

  /**
   * Deletes a contact.
   */
  public function deleteContact($contact_iden) {
    return $this->curlRequest(self::URL_CONTACTS . '/' .
      $contact_iden, 'DELETE');
  }

  /**
   * Gets information about the user.
   */
  public function getUserInformation($user = 'me') {
    return $this->curlRequest(self::URL_USERS . '/' . $user, 'GET');
  }

  /**
   * Processing the push.
   */
  protected function push($recipient, $type, $arg1, $arg2 = NULL, $arg3 = NULL) {
    $query_data = array();

    if (!empty($recipient)) {
      if (filter_var($recipient, FILTER_VALIDATE_EMAIL) !== FALSE) {
        $query_data['email'] = $recipient;
      }
      else {
        $query_data['device_iden'] = $recipient;
      }
    }

    $query_data['type'] = $type;

    switch ($type) {
      case 'note':
        $query_data['title'] = $arg1;
        $query_data['body']  = $arg2;
        break;

      case 'link':
        $query_data['title'] = $arg1;
        $query_data['url']   = $arg2;

        if ($arg3 !== NULL) {
          $query_data['body'] = $arg3;
        }
        break;

      case 'address':
        $query_data['name']    = $arg1;
        $query_data['address'] = $arg2;
        break;

      case 'list':
        $query_data['title'] = $arg1;
        $query_data['items'] = $arg2;
        break;

      case 'file':
        $full_file_path = realpath($arg1);

        if (!is_readable($full_file_path)) {
          throw new PushbulletPushBulletException('File: File does not exist ' .
            'or is unreadable.');
        }

        if (filesize($full_file_path) > 25 * 1024 * 1024) {
          throw new PushbulletPushBulletException('File: File size exceeds 25 MiB.');
        }

        $query_data['file_name'] = basename($full_file_path);

        // Try to guess the MIME type if the argument is NULL.
        if ($arg2 === NULL) {
          $query_data['file_type'] = mime_content_type($full_file_path);
        }
        else {
          $query_data['file_type'] = $arg2;
        }

        // Request authorization to upload a file.
        $response = $this->curlRequest(self::URL_UPLOAD_REQUEST, 'GET',
          $query_data);
        $query_data['file_url'] = $response->file_url;

        // Upload the file.
        $response->data->file = '@' . $full_file_path;
        $this->curlRequest($response->upload_url, 'POST', $response->data,
          FALSE, FALSE);
        break;

      default:
        throw new PushbulletPushBulletException('Unknown push type.');
    }

    return $this->curlRequest(self::URL_PUSHES, 'POST', $query_data);
  }

  /**
   * Makes a curl request.
   */
  protected function curlRequest($url, $method, $data = NULL,
    $send_as_json = TRUE, $auth = TRUE) {
    $curl = curl_init();

    if ($method == 'GET' && $data !== NULL) {
      $url .= '?' . http_build_query($data);
    }

    curl_setopt($curl, CURLOPT_URL, $url);

    if ($auth) {
      curl_setopt($curl, CURLOPT_USERPWD, $this->apiKey);
    }

    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, $method);

    if ($method == 'POST' && $data !== NULL) {
      if ($send_as_json) {
        $data = json_encode($data);
        curl_setopt($curl, CURLOPT_HTTPHEADER, array(
          'Content-Type: application/json',
          'Content-Length: ' . strlen($data),
        ));
      }

      curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
    }

    curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($curl, CURLOPT_HEADER, FALSE);

    $response = curl_exec($curl);

    if ($response === FALSE) {
      $curl_error = curl_error($curl);
      curl_close($curl);
      throw new PushbulletPushBulletException('cURL Error: ' . $curl_error);
    }

    $http_code = curl_getinfo($curl, CURLINFO_HTTP_CODE);

    if ($http_code >= 400) {
      curl_close($curl);
      throw new PushbulletPushBulletException('HTTP Error ' . $http_code);
    }

    curl_close($curl);

    return json_decode($response);
  }
}

class PushbulletPushBulletException extends Exception {
  // Exception thrown by PushBullet.
}
