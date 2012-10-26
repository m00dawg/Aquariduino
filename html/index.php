<html>
<head>
<title>Aquarium</title>
<link rel="stylesheet" href="/global.css" type="text/css" />
</head>
<body>

<?php
include 'config.inc.php';

$link = mysql_connect($mysql['host'], $mysql['user'], $mysql['password']);
mysql_select_db($mysql['database']);

$result = mysql_query("SELECT DATE(testedOn) AS testedOn, temperature, ammonia, 
	nitrites, nitrates, pH, KH, amountExchanged, comments
	FROM WaterLog ORDER BY testedOn");

echo "<h2>Water Log</h2>\n";
echo "<table>\n";
echo "<tr>";
echo "<th>Tested On</th>";
echo "<th>Ammonia</th>";
echo "<th>Nitrites</th>";
echo "<th>Nitrates</th>";
echo "<th>pH</th>";
echo "<th>KH</th>";
echo "<th>Litters Exchanged</th>";
echo "<th>Comments</th>";
echo "</tr>\n";
while($row = mysql_fetch_assoc($result))
{
	echo "<tr><td>";
	echo $row['testedOn'];
	echo "</td><td>";
	echo $row['ammonia'];
	echo "</td><td>";
	echo $row['nitrites'];
	echo "</td><td>";
	echo $row['nitrates'];
	echo "</td><td>";
	echo $row['pH'];
	echo "</td><td>";
	echo $row['KH'];
	echo "</td><td>";
	echo $row['amountExchanged'];
	echo "</td><td>";
	echo $row['comments'];
	echo "</td></tr>\n";
}
echo "</table>\n";

$result = mysql_query("SELECT Equipment.name AS name, 
DATE(logDate) AS logDate, maintenance, action
FROM Equipment
JOIN EquipmentLog ON EquipmentLog.equipmentID = Equipment.equipmentID
ORDER BY logDate");

echo "<h2>Equipment Log</h2>\n";
echo "<table>\n";
echo "<tr>";
echo "<th>Equipment</th>";
echo "<th>Date</th>";
echo "<th>Maintenance</th>";
echo "<th>Action</th>";
echo "</tr>\n";

while($row = mysql_fetch_assoc($result))
{
	echo "<tr><td>";
	echo $row['name'];
	echo "</td><td>";
	echo $row['logDate'];
	echo "</td><td>";
	echo $row['maintenance'];
	echo "</td><td>";
	echo $row['action'];
	echo "</td></tr>\n";
}
echo "</table>\n";

mysql_close();

?>

</body>
</html>
