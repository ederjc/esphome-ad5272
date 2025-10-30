import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_OHM,
)
from . import ad5272_ns, AD5272Component, CONF_AD5272_ID

DEPENDENCIES = ['ad5272']

AD5272Sensor = ad5272_ns.class_('AD5272Sensor', sensor.Sensor, cg.Component)

CONF_TYPE = 'type'

SENSOR_TYPES = {
    'resistance': {
        'icon': 'mdi:resistor',
        'unit': UNIT_OHM,
        'accuracy_decimals': 1,
        'device_class': DEVICE_CLASS_EMPTY,
        'state_class': STATE_CLASS_MEASUREMENT,
    },
    'wiper_position': {
        'icon': 'mdi:tune-vertical',
        'unit': '',
        'accuracy_decimals': 0,
        'device_class': DEVICE_CLASS_EMPTY,
        'state_class': STATE_CLASS_MEASUREMENT,
    },
}

CONFIG_SCHEMA = sensor.sensor_schema(
    AD5272Sensor,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_EMPTY,
    state_class=STATE_CLASS_MEASUREMENT,
).extend({
    cv.GenerateID(CONF_AD5272_ID): cv.use_id(AD5272Component),
    cv.Required(CONF_TYPE): cv.enum(SENSOR_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_AD5272_ID])
    cg.add(var.set_parent(parent))
    
    sensor_type = config[CONF_TYPE]
    cg.add(var.set_sensor_type(cg.const_char_ptr(sensor_type)))
    
    # Apply sensor type specific settings
    type_config = SENSOR_TYPES[sensor_type]
    if 'icon' in type_config and 'icon' not in config:
        cg.add(var.set_icon(type_config['icon']))
    if 'unit' in type_config and type_config['unit'] and 'unit_of_measurement' not in config:
        cg.add(var.set_unit_of_measurement(type_config['unit']))