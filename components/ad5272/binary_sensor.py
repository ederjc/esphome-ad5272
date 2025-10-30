import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import ad5272_ns, AD5272Component, CONF_AD5272_ID

DEPENDENCIES = ['ad5272', 'binary_sensor']

AD5272BinarySensor = ad5272_ns.class_('AD5272BinarySensor', binary_sensor.BinarySensor, cg.Component)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    AD5272BinarySensor
).extend({
    cv.GenerateID(CONF_AD5272_ID): cv.use_id(AD5272Component),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await binary_sensor.register_binary_sensor(var, config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_AD5272_ID])
    cg.add(var.set_parent(parent))