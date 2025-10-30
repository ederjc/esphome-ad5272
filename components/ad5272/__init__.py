import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ['i2c']

ad5272_ns = cg.esphome_ns.namespace('ad5272')
AD5272Component = ad5272_ns.class_('AD5272Component', cg.Component, i2c.I2CDevice)

CONF_AD5272_ID = 'ad5272_id'
CONF_MAX_RESISTANCE = 'max_resistance'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AD5272Component),
    cv.Optional(CONF_MAX_RESISTANCE, default=20000): cv.float_range(min=1000, max=100000),
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x2F))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    cg.add(var.set_max_resistance(config[CONF_MAX_RESISTANCE]))

# Actions
SetResistanceAction = ad5272_ns.class_('SetResistanceAction', cg.Action)

@cg.register_action("ad5272.set_resistance", SetResistanceAction, cv.Schema({
    cv.GenerateID(): cv.use_id(AD5272Component),
    cv.Required("resistance"): cv.templatable(cv.float_range(min=0)),
}))
async def ad5272_set_resistance_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config["resistance"], args, float)
    cg.add(var.set_resistance(template_))
    return var