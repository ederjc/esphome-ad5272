import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_ICON
from . import ad5272_ns, AD5272Component, CONF_AD5272_ID

DEPENDENCIES = ['ad5272']

AD5272TextSensor = ad5272_ns.class_('AD5272TextSensor', text_sensor.TextSensor, cg.Component)

CONFIG_SCHEMA = text_sensor.text_sensor_schema(
    AD5272TextSensor
).extend({
    cv.GenerateID(CONF_AD5272_ID): cv.use_id(AD5272Component),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)
    
    parent = await cg.get_variable(config[CONF_AD5272_ID])
    cg.add(var.set_parent(parent))