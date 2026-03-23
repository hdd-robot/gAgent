# Configuration Sphinx pour gAgent
import os
import subprocess

# -- Informations projet -------------------------------------------------------

project   = 'gAgent'
copyright = '2025, HD — Université Paris 8'
author    = 'HD'
release   = '0.9.0'
version   = '0.9'

# -- Extensions ----------------------------------------------------------------

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.githubpages',
    'sphinx.ext.todo',
    'sphinx.ext.graphviz',
    'breathe',
]

# -- Breathe (pont Doxygen → Sphinx) ------------------------------------------

breathe_projects = {
    'gAgent': os.path.join(os.path.dirname(__file__), '_doxygen/xml')
}
breathe_default_project = 'gAgent'
breathe_default_members = ('members', 'undoc-members')

# -- Options générales ---------------------------------------------------------

templates_path   = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
language         = 'fr'
todo_include_todos = True

# -- HTML ----------------------------------------------------------------------

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

html_theme_options = {
    'navigation_depth': 4,
    'collapse_navigation': False,
    'sticky_navigation': True,
    'includehidden': True,
    'titles_only': False,
}

html_context = {
    'display_github': False,
}

# -- Génération automatique de Doxygen avant le build Sphinx ------------------

def run_doxygen(app):
    doxy_dir  = os.path.join(os.path.dirname(__file__), '..')
    doxy_file = os.path.join(doxy_dir, 'Doxyfile.sphinx')
    if os.path.exists(doxy_file):
        subprocess.run(['doxygen', doxy_file], cwd=doxy_dir, check=True)

def setup(app):
    app.connect('builder-inited', run_doxygen)
