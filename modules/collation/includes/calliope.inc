<?php

/**
 * @file
 * Handles Calliope's Mongo interaction.
 */

/**
 * Creates configured MongoDB object.
 *
 * @return MongoDB
 *   MongoDB built with Islandora configuration
 */
function emicdora_calliope_create_mongo_db() {
  $dbhost = variable_get('emicdora_mongo_host', "localhost");
  $dbport = variable_get('emicdora_mongo_port', "27017");
  $dbname = variable_get('emicdora_mongo_db', "test");
  try {
    $mongo = new MongoClient("$dbhost:$dbport");
    $db = $mongo->selectDB($dbname);
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage());
    exit;
  }
  return $db;
}

/**
 * Creates MongoFSGrid.
 *
 * @return MongoFSGrid
 *   MongoFSGrid using supplied credentials
 */
function emicdora_calliope_create_mongo_grid() {
  try {

    $db = emicdora_calliope_create_mongo_db();
    $grid = $db->getGridFS();
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage());
    exit;
  }
  return $grid;
}

/**
 * Adds Transcription record to Mongo.
 *
 * @param string $pid
 *   PID of Transcription Object
 * @param string $critical_edition
 *   Optional project associated with transcription
 */
function emicdora_calliope_write_file($pid, $critical_edition = '') {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $transcription_object = islandora_object_load($pid);
  $transcription_ds = $transcription_object['TRANSCRIPTION'];
  if (!$transcription_ds) {
    drupal_set_message(t("@pid has no transcription datastream", array('@pid' => $pid)));
    return;
  }
  $flat_pid = islandora_escape_pid_for_function($pid);
  $filename = file_create_filename($flat_pid . "_temp_for_mongo", 'temporary://');
  $real_path = drupal_realpath($filename);
  $contents = $transcription_ds->content;
  file_put_contents($filename, $contents);
  try {
    $grid = emicdora_calliope_create_mongo_grid();
    $query = array('_resourceid' => $pid);
    $record = $grid->findOne($query);
    if ($record) {
      $id = $record->file['_id'];
      $grid->remove($query);
    }
    $record2 = $grid->findOne($query);
    $metadata['filetype'] = $transcription_ds->mimetype;
    if ($critical_edition) {
      $metadata['project'] = $critical_edition;
    }

    $storedfile = $grid->storeFile($real_path, array('metadata' => $metadata));
    $id = $storedfile->{'$id'};
    $grid->update(
        array('_id' => new MongoId($id)), array('$set' => array('_resourceid' => $pid, 'filename' => $transcription_object->label)), array('w' => 1));
    unlink($filename);
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage());
    exit;
  }
}

/**
 * Deletes Mongo record assoicated with pid.
 *
 * @param string $pid
 *   PID of associated object
 */
function emicdora_calliope_delete_file($pid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  try {
    emicdora_calliope_delete_file_no_exit($pid);
  }
  catch (Exception $e) {
    drupal_add_http_header('Status', '500 Internal server error');
    echo $e->getMessage();
    exit;
  }
}

/**
 * Deletes Mongo record associated with pid.
 *
 * @throws Exception
 *
 * @param string $pid
 *   PID of associated object
 */
function emicdora_calliope_delete_file_no_exit($pid) {
  $grid = emicdora_calliope_create_mongo_grid();
  $query = array('_resourceid' => $pid);
  $record = $grid->findOne($query);
  $id = isset($record->file['_id']) ? $record->file['_id'] : FALSE;
  if ($id) {
    $grid->remove($query);
  }
}

/**
 * Deletes Mongo collation records.
 *
 * @param string $name
 *   Name of collation to kill.
 */
function emicdora_calliope_delete_collation($name) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  try {
    emicdora_calliope_delete_collation_no_exit($name);
  }
  catch (Exception $e) {
    drupal_add_http_header('Status', '500 Internal server error');
    echo $e->getMessage();
    exit;
  }
}

/**
 * Deletes Mongo collation records.
 *
 * @param string $name
 *   Name of collation to kill.
 */
function emicdora_calliope_delete_collation_no_exit($name) {
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('collations');
  $collection->remove(array('emic_data.collation' => $name));
}

/**
 * Deletes Mongo MVD record.
 *
 * @param string $id
 *   Mongo ID of MVD
 */
function emicdora_calliope_delete_mvd($id) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  try {
    emicdora_calliope_delete_mvd_no_exit($id);
  }
  catch (Exception $e) {
    drupal_add_http_header('Status', '500 Internal server error');
    echo $e->getMessage();
    exit;
  }
}

/**
 * Deletes Mongo MVD record.
 *
 * @throws Exception
 *
 * @param string $id
 *   Mongo ID of MVD
 */
function emicdora_calliope_delete_mvd_no_exit($id) {
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('mvds');
  $collection->remove(array('_id' => $id));
}

/**
 * Send selected documents to calliope to create MVD.
 *
 * @global type $base_url
 *
 * @param string $critical_edition
 *   MVD's Project
 * @param string $mvd_title
 *   Title for MVD
 * @param array $resource_ids
 *   Array of transcriptions to build MVD
 * @param string $filter
 *   Optional Filter
 *
 * @return string
 *   Status message
 */
function emicdora_calliope_create_mvd($critical_edition, $mvd_title, $mvd_description, $resource_ids = NULL, $filter = "Empty") {
  global $base_url;
  $mvd_success = FALSE;
  // We need a one character token that won't appear in normal text.
  $br_token = json_decode('"\uFFFC"');
  $mvd_url = "$base_url/calliope";
  $calliope_import_url = "$mvd_url/import/";
  $verb = t('created');

  // Remove spaces from doc path.
  $docpath = str_replace(' ', '_', "$mvd_title");
  $origdocpath = $docpath;
  if ($resource_ids == NULL) {
    return t('Identifiers of resources to add to MVD not specified');
  }
  $boundary = "-------------" . uniqid();
  $data = "";
  $id_array = array();
  foreach ($resource_ids as $resource_id) {
    if ($resource_id != "") {
      $file = emicdora_calliope_get_file($resource_id);
      if (!$file) {
        emicdora_calliope_write_file($resource_id);
        $file = emicdora_calliope_get_file($resource_id);
        if (!$file) {
          drupal_set_message(t("Resource_id could not be used to make file"));
          return;
        }
      }
      // Use nl2br's ability to harvest all line endings.
      $contents = nl2br($file->getBytes());
      $contents = str_replace('<br />', $br_token, $contents);
      $filename = $file->file['filename'];
      $file_label = str_replace('.', '_', pathinfo($filename, PATHINFO_FILENAME));
      $id_array[] = array('id' => $resource_id, 'name' => $file_label);

      $data .= "--" . $boundary . "\r\n";
      $data .= "Content-Disposition: form-data; name=\"uploadedfile[]\"; filename=\"" . $file_label . "\"\r\n";
      $data .= "Content-Type: " . 'text/xml' . "\r\n";
      $data .= "Content-Length: " . strlen($contents) . "\r\n";
      $data .= "Content-Transfer-Encoding: binary\r\n\r\n";
      $data .= $contents . "\r\n";
    }
  }
  $data .= "--" . $boundary . "\r\n";
  $data .= "Content-Disposition: form-data; name=\"DOC_ID\"\r\n\r\n" . $docpath . "\r\n";
  $data .= "--" . $boundary . "--\n";

  $data .= "--" . $boundary . "\r\n";
  $data .= "Content-Disposition: form-data; name=\"FILTER\"\r\n\r\n" . $filter . "\r\n";
  $data .= "--" . $boundary . "--\n";

  $ch = curl_init();
  // Post to Calliope server import.
  curl_setopt($ch, CURLOPT_URL, trim($calliope_import_url));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array("Content-type: multipart/form-data; boundary=" . $boundary));
  // Get back status or response.
  $response = curl_exec($ch);
  $err = curl_errno($ch);
  $error = curl_error($ch);
  $contentType = curl_getinfo($ch, CURLINFO_CONTENT_TYPE);
  curl_close($ch);
  if ($err) {
    drupal_set_message(t("Error @err @error @response", array(
      '@err' => $err,
      '@error' => $error,
      '@response' => $response,
    )));
    return FALSE;
  }
  else {
    // Unfortunately HRIT Server does not use HTTP status to indicate errors.
    // Look for 'rror' in response instead.
    $hriterror = preg_match('/rror/', $response);
    $rejected = preg_match('/[rR]eject/', $response);
    if ($hriterror == 0 && $rejected == 0) {
      // Try to fetch json for created mvd.
      $hriterror = preg_match('/HritServer Error/', $response);
      if ($hriterror == 0) {
        $query = array(
          'metadata.critical_edition' => $critical_edition,
          'metadata.name' => $docpath,
        );
        $mvds = emicdora_calliope_find_mvd($query);
        if (!empty($mvds)) {
          foreach ($mvds as $mvd) {
            emicdora_calliope_delete_mvd($mvd['_id']);
          }
          $verb = t('updated');
          emicdora_calliope_delete_collation($mvd_title);
        }
        // Post new MVD details to db.
        $newmvd = array(
          'name' => $origdocpath,
          'resources' => $id_array,
          'filter' => $filter,
          'critical_edition' => $critical_edition,
          'description' => $mvd_description,
        );
        $mvd_success = emicdora_calliope_write_mvd($newmvd);
      }
    }
    else if ($contentType == 'text/html;charset=UTF-8') {
      $doc = new DOMDocument(1.0, 'UTF-8');
      $doc->loadHTML($response);
      $xpath = new DOMXPath($doc);
      $results = $xpath->query('//text()[contains(., "Reject") or contains(., "reject")]');
      if ($results->length > 0) {
        $message = '';
        foreach ($results as $node) {
          $message .= $doc->saveXML($node);
        }
        drupal_set_message($message);
      }
    }
    if ($mvd_success) {
      drupal_set_message(t("Collation successfully @verb.", array('@verb' => $verb)));
    }
  }
  return $mvd_success;
}

/**
 * Returns Mongo file associated with pid.
 *
 * @param string $pid
 *   PID of Fedora object with associated Mongo file
 *
 * @return mixed
 *   Associated MongoGridFSFile file if found, otherwise bool FALSE
 */
function emicdora_calliope_get_file($pid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $id = islandora_escape_pid_for_function($pid);
  $grid = emicdora_calliope_create_mongo_grid();
  if (!$grid) {
    return FALSE;
  }
  $query = array('_resourceid' => $pid);
  $file = $grid->findOne($query);
  if ($file == NULL) {
    return FALSE;
  }
  if (array_key_exists('_deleted', $file->file)) {
    drupal_set_message(t('The requested resource has been deleted'));
    return;
  }
  return $file;
}

/**
 * Pulls all MVD metadata from Mongo.
 *
 * @param AbstractObject $critical_edition_object
 *   Object representing Critical Edition
 * @param string $sort
 *   Sort type
 * @param int $limit
 *   Size of resultset
 * @param int $offset
 *   Offset for db call
 *
 * @return array
 *   Associative array of MVD data
 */
function emicdora_calliope_find_mvds($critical_edition_object, $sort = "asc", $limit = NULL, $offset = 0) {
  try {
    return emicdora_calliope_find_mvds_no_exit($critical_edition_object, $sort, $limit, $offset);
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage());
    exit;
  }
}

/**
 * Pulls all MVD metadata from Mongo.
 *
 * @throws Exception
 *
 * @param AbstractObject $critical_edition_object
 *   Object representing Critical Edition
 * @param string $sort
 *   Sort type
 * @param int $limit
 *   Size of result set.
 * @param int $offset
 *   Offset for db call
 *
 * @return array
 *   Associative array of MVD data
 */
function emicdora_calliope_find_mvds_no_exit($critical_edition_object, $sort = "asc", $limit = NULL, $offset = 0) {
  $query = array('metadata.critical_edition' => $critical_edition_object->id);
  $sort = strtolower($sort);
  $sort_val = ($sort == 'asc') ? 1 : -1;
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('mvds');
  $mvd_cursor = $collection->find($query);
  $mvd_cursor->sort(array('metadata.name' => $sort_val));
  $mvd_data = array();
  foreach ($mvd_cursor as $mvd) {
    if (is_array($mvd['metadata']['resources'])) {
      $transcriptions = array();
      foreach ($mvd['metadata']['resources'] as $transcription) {
        $transcriptions[$transcription['id']] = $transcription['name'];
      }
      $mvd_data[$mvd['metadata']['name']]['name'] = $mvd['metadata']['name'];
      $mvd_data[$mvd['metadata']['name']]['filter'] = $mvd['metadata']['filter'];
      $mvd_data[$mvd['metadata']['name']]['transcriptions'] = $transcriptions;
      if (isset($mvd['metadata']['description'])) {
        $mvd_data[$mvd['metadata']['name']]['description'] = $mvd['metadata']['description'];
      }
      else {
        $mvd_data[$mvd['metadata']['name']]['description'] = t("No description");
      }
    }
  }
  return $mvd_data;
}

/**
 * Pulls all MVD metadata from Mongo.
 *
 * @param array $query
 *   Mongo query
 *
 * @return array
 *   All values associated with all MVD's attached to this Critical Edition
 */
function emicdora_calliope_find_mvd($query) {
  try {
    return emicdora_calliope_find_mvd_no_exit($query);
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage());
    exit;
  }
}

/**
 * Pulls all MVD metadata from Mongo.
 *
 * @param array $query
 *   Mongo query
 *
 * @return array
 *   All values associated with all MVD's attached to this Critical Edition
 */
function emicdora_calliope_find_mvd_no_exit($query) {
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('mvds');
  $mvd_cursor = $collection->find($query);
  $mvd_data = array();
  foreach ($mvd_cursor as $mvd) {
    $mvd_data[] = $mvd;
  }
  return $mvd_data;
}

/**
 * Writes new MVD to Mongo.
 *
 * @param array $newmvd
 *   All paramaters to build new MVD
 *
 * @return bool
 *   Success indicator
 */
function emicdora_calliope_write_mvd($newmvd) {
  $success = FALSE;
  $newobj = array('_revisions' => array($newmvd), 'metadata' => $newmvd);
  $db = emicdora_calliope_create_mongo_db();
  $coll = $db->selectCollection('mvds');
  $inserted = $coll->insert($newobj, array('w' => 1));
  if ($inserted) {
    $success = TRUE;
  }
  return $inserted;
}

/**
 * Brings back basic mvd information filtered by Critical Edition.
 *
 * @param string $critical_edition_pid
 *   PID of critical edition
 *
 * @return MongoCursor
 *   Query results
 */
function emicdora_calliope_compare_callback($critical_edition_pid) {
  $query = array('metadata.critical_edition' => $critical_edition_pid);
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('mvds');
  return $collection->find($query);
}

/**
 * Writes new Collation to Mongo.
 *
 * @param array $contents
 *   All paramaters to build new MVD
 *
 * @return bool
 *   Success indicator
 */
function emicdora_calliope_write_collation($query, $contents) {
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('collations');
  $search_query = array(
    'emic_data.collation' => $query['collation'],
    'emic_data.diff_kind' => $query['diff_kind'],
    'emic_data.version1' => $query['version1'],
    'emic_data.version2' => $query['version2'],
  );
  $current_record = emicdora_calliope_get_collation_file($search_query);
  if ($current_record) {
    $collection->remove(array('_id' => $current_record['_id']));
  }
  $query['contents'] = $contents;
  $newobj = array('emic_data' => $query);
  $inserted = $collection->insert($newobj, array('w' => 1));
  return $inserted ? TRUE : FALSE;
}

/**
 * Returns Mongo file associated with query.
 *
 * @param string $query
 *   Mongo query
 *
 * @return MongoGridFSFile
 *   Associated file
 */
function emicdora_calliope_get_collation_file($query) {
  $db = emicdora_calliope_create_mongo_db();
  $collection = $db->selectCollection('collations');
  $file = $collection->findOne($query);
  return $file;
}
