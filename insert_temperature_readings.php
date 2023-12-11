<?php

if(isset($_GET["temperature"])) {
   $temperature = $_GET["temperature"]; // get sound value from HTTP GET

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

   $sql = "INSERT INTO  tbl_temperature_readings (insertDateTime, temperature_value) VALUES (now(), $temperature)";

   if ($conn->query($sql) === TRUE) {
      echo "New record created successfully";
   } else {
      echo "Error: " . $sql . " => " . $conn->error;
   }

   $conn->close();
} else {
   echo "temperature is not set";
}
?>
