project('moltenUI', 'c',
  version: '0.1.0',
  default_options: [
    'c_std=c99',
    'warning_level=2',
  ]
)

fonts = [
  'SF-Pro-Rounded-Regular.otf',
]

vulkan_dep = dependency('vulkan', required: true)
glfw_dep = dependency('glfw3', required: true)
freetype_dep = dependency('freetype2', required: true)

glslc = find_program('glslc', required: true)

inc_dirs = include_directories('include')

text_frag_spv = custom_target('text_frag',
  input: 'shaders/text.frag',
  output: 'text.frag.spv',
  command: [glslc, '@INPUT@', '-o', '@OUTPUT@'],
  install: true,
  install_dir: get_option('datadir') / 'moltenUI' / 'shaders'
)

text_vert_spv = custom_target('text_vert',
  input: 'shaders/text.vert',
  output: 'text.vert.spv',
  command: [glslc, '@INPUT@', '-o', '@OUTPUT@'],
  install: true,
  install_dir: get_option('datadir') / 'moltenUI' / 'shaders'
)

foreach font : fonts
  configure_file(
    input: 'fonts' / font,
    output: font,
    copy: true,
    install_dir: get_option('datadir') / 'moltenUI' / 'fonts'
  )
endforeach

executable('moltenUI',
  'src/main.c',
  sources: [
    'src/loop.c',
    'src/command.c',
    'src/device.c',
    'src/framebuffer.c',
    'src/instance.c',
    'src/pipeline.c',
    'src/renderpass.c',
    'src/swapchain.c',
    'src/sync.c',
    'src/window.c',
    'src/font.c'
  ],
  include_directories: inc_dirs,
  dependencies: [vulkan_dep, glfw_dep, freetype_dep],
  install: true
)
