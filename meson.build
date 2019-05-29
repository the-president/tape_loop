project('seedl', 'cpp',default_options:'cpp_std=c++17')
sdl_dep = dependency('sdl2')
sdl_img = dependency('sdl2_image')
sdl_ttf = dependency('sdl2_ttf')
rtaudio = dependency('librtaudio')
libsndfile = dependency('sndfile')

src = [
	'main.cpp',
	'sdl_system.cpp',
	'frame_rate_manager.cpp',
	'audio.cpp',
	'graphical_interface.cpp',
	'bmp_console.cpp',
	'tape_lop.cpp',
	'tape_lop_screen.cpp',
	'audio_select_screen.cpp',
	'state_variable_filter.cpp',
	'tape_lop_gui.cpp',
	'file_dialog/tinyfiledialogs.cpp'
]

executable('tape_lop', src,dependencies:[sdl_dep,sdl_img,sdl_ttf,rtaudio,libsndfile])
