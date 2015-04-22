
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

import os.path

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf='{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
        target=app_elf)

        if build_worker:
            worker_elf='{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': p, 'app_elf': app_elf, 'worker_elf': worker_elf})
            ctx.pbl_worker(source=ctx.path.ant_glob('worker_src/**/*.c'),
            target=worker_elf)
        else:
            binaries.append({'platform': p, 'app_elf': app_elf})

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

    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries, js=build_js)
