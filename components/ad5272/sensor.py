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

SENSOR_TYPE_RESISTANCE = "resistance"
SENSOR_TYPE_WIPER_POSITION = "wiper_position"

SENSOR_TYPES = [SENSOR_TYPE_RESISTANCE, SENSOR_TYPE_WIPER_POSITION]

CONFIG_SCHEMA = sensor.sensor_schema(
    AD5272Sensor,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_EMPTY,
    state_class=STATE_CLASS_MEASUREMENT,
).extend({
    cv.GenerateID(CONF_AD5272_ID): cv.use_id(AD5272Component),
    cv.Required(CONF_TYPE): cv.one_of(*SENSOR_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_AD5272_ID])
    cg.add(var.set_parent(parent))
    
    sensor_type = config[CONF_TYPE]
    cg.add(var.set_sensor_type(sensor_type))
    
    # Apply sensor type specific settings based on type
    if sensor_type == SENSOR_TYPE_RESISTANCE:
        if 'icon' not in config:
            cg.add(var.set_icon('mdi:resistor'))
        if 'unit_of_measurement' not in config:
            cg.add(var.set_unit_of_measurement(UNIT_OHM))
    elif sensor_type == SENSOR_TYPE_WIPER_POSITION:
        if 'icon' not in config:
            cg.add(var.set_icon('mdi:tune-vertical'))