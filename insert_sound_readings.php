<?php

if(isset($_GET["sound"])) {
   $sound = $_GET["sound"]; // get sound value from HTTP GET

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

   $sql = "INSERT INTO  tbl_sound_readings (insertDateTime, sound_value) VALUES (now(), $sound)";

   if ($conn->query($sql) === TRUE) {
      echo "New record created successfully";
   } else {
      echo "Error: " . $sql . " => " . $conn->error;
   }

   $conn->close();
} else {
   echo "sound is not set";
}
?>
