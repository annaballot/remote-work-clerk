<?php

   $servername = "localhost";
   $username = "Arduino";
   $password = "pass";
   $dbname = "db_arduino";

   // Create connection
   $conn = new mysqli($servername, $username, $password, $dbname);
   // Check connection
   if ($conn->connect_error) {
      die("Connection failed: " . $conn->connect_error);
   }


// Perform query
if ($result = $conn -> query("SELECT AVG(sound_value) as avgSoundVal FROM tbl_sound_readings")) {
$row = mysqli_fetch_array($result, MYSQLI_ASSOC);

  echo json_encode($row);


   // Free result set
   $result -> free_result();
 }

   $conn->close();

?>
