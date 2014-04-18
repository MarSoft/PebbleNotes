# vim: ft=python
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
    # syntax-check appinfo json before loading
    if ctx.exec_command('jshint appinfo.json'):
        ctx.fatal("Errors in appinfo.json")

    ctx.load('pebble_sdk')

    ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
                    target='pebble-app.elf')

    # Join all JS files (first subdirs, then root, then main.js):
    # get all js files..
    src_js = ctx.path.ant_glob('src/js/**/*.js')
    # get main.js node
    src_js_main = ctx.path.make_node('src/js/main.js')
    # move that node to end of list
    src_js.remove(src_js_main)
    src_js.append(src_js_main)
    # get destination path for joined file
    build_js = ctx.path.get_bld().make_node('src/js/pebble-js-app.js')
    # check syntax (jshint) and minify (uglifyjs)
    ctx(rule='(echo ${SRC}; jshint ${SRC} && uglifyjs ${SRC} -o ${TGT})',
        source=src_js, target=build_js)

    ctx.pbl_bundle(elf='pebble-app.elf', js=build_js)
