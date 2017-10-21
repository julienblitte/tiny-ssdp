<?php

require_once('View.class.php');

define('ICON_DIR', 'icon');
define('CONFIG_FILE', '/etc/ssdpd');

function assert_config($config, $variable, $numeric=false)
{
	if (!isset($config[$variable]))
	{
		trigger_error('Variable '.htmlspecialchars($variable).' in "'.htmlspecialchars(CONFIG_FILE).'" is not defined.', E_USER_ERROR);
	}
	if ($numeric && !is_numeric($config[$variable]))
	{
		trigger_error('Variable '.htmlspecialchars($variable).' in "'.htmlspecialchars(CONFIG_FILE).'" is not a number.', E_USER_ERROR);
	}
}

$config = parse_ini_file(CONFIG_FILE);
assert_config($config, 'base_url_port', true);
assert_config($config, 'base_url_resource');
assert_config($config, 'device_type_name');
assert_config($config, 'device_type_version');
assert_config($config, 'device_description');
assert_config($config, 'device_uuid');
assert_config($config, 'service_count');
assert_config($config, 'manufacturer_name');
assert_config($config, 'manufacturer_url');
assert_config($config, 'friendly_name');
assert_config($config, 'model_name');
assert_config($config, 'model_number');
assert_config($config, 'model_url');

/* iconList */
$iconList = '';
if (is_dir(ICON_DIR))
{
	$v = new View('ssdp.icon.xml');
	$icons = scandir(ICON_DIR);
	foreach($icons as $icon)
	{
		$icon = ICON_DIR.DIRECTORY_SEPARATOR.$icon;
		if (is_file($icon))
		{
			$param = array();
			list($param['width'], $param['height']) = getimagesize($icon);
			$param['mimetype'] = mime_content_type($icon);
			$param['depth'] = 24;
			$param['icon'] = $icon;

			$iconList .= $v->get($param);
		}
	}
}

/* serviceList */
$serviceList = '';
$v = new View('ssdp.service.xml');
for($i=0; $i < $config['service_count']; $i++)
{
	$service_name = (isset($config['service_type_name'][$i]) ?
		$config['service_type_name'][$i] : 'MyService'.$i
		);
	$service_version = (isset($config['service_type_version'][$i]) ?
		$service_version = $config['service_type_version'][$i] : '1'
		);

	$param = array();
	$param['serviceType'] = 'urn:schemas-upnp-org:service:'.$service_name.':'.$service_version;
	$param['serviceId'] = 'urn:schemas-upnp-org:service:'.$service_name.':'.$service_version;
	$param['controlURL'] = '/service/'.$service_name.'.php?version='.urlencode($service_version).'&amp;action=control';
	$param['eventSubURL'] = '/service/'.$service_name.'.php?version='.urlencode($service_version).'&amp;action=eventSub';
	$param['SCPDURL'] = '/service/'.$service_name.'.xml';

	$serviceList .= $v->get($param);
}

/* URLBase */
$URLBase='http://'.$_SERVER['SERVER_ADDR'].':'.$config['base_url_port'].$config['base_url_resource'];
/* deviceType */
$deviceType='urn:schemas-upnp-org:device:'.$config['device_type_name'].':'.$config['device_type_version'];
/* manufacturer, manufacturerURL */
$manufacturer = $config['manufacturer_name'];
$manufacturerURL = $config['manufacturer_url'];
/* friendlyName */
$friendlyName = $config['friendly_name'];
/* modelName, modelNumber, modelURL */
$modelName = $config['model_name'];
$modelNumber = $config['model_number'];
$modelURL = $config['model_url'];

$device_description_parts = explode(',', $config['device_description']);
if (isset($device_description_parts[2]))
{
	$model_parts = explode('/', $device_description_parts[2]);
	$modelName = trim($model_parts[0]);
	if (isset($model_parts[1]))
		$modelNumber = trim($model_parts[1]);

}
/* modelDescription */
$modelDescription=$modelName.' '.$modelNumber;
/* serialNumber */
$serialNumber=$config['device_uuid'];
/* UDN */
$UDN='uuid:'.$config['device_uuid'];
/* presentationURL */
$presentationURL='http://'.$_SERVER['SERVER_ADDR'].':'.$config['base_url_port'].'/?presentation=1';

if (isset($_GET['presentation']))
{
	$v = new View('presentation.html');
	header("Content-type: text/html");
	$v->show();
}
else
{
	$v = new View('ssdp.xml');
	header("Content-type: text/xml");
	$v->show();
}

?>
