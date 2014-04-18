
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
                    target='pebble-app.elf')

    # Join all JS files
    src_js = ctx.path.ant_glob('src/js/**/*.js')
    build_js = ctx.path.get_bld().make_node('src/js/pebble-js-app.js')
    ctx(rule='(test -e ${TGT} && rm ${TGT}; jshint ${SRC} && cat ${SRC} >> ${TGT})',
        source=src_js, target=build_js)

    ctx.pbl_bundle(elf='pebble-app.elf',
                   js=ctx.path.ant_glob('src/js/**/*.js'))
