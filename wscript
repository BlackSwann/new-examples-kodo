#! /usr/bin/env python
# encoding: utf-8

def recurse_helper(ctx, name):
    if not ctx.has_dependency_path(name):
        ctx.fatal('Load a tool to find %s as system dependency' % name)
    else:
        p = ctx.dependency_path(name)
        ctx.recurse(p)


def options(opt):
    import waflib.extras.wurf_dependency_bundle as bundle
    import waflib.extras.wurf_dependency_resolve as resolve

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='boost',
        git_repository='github.com/steinwurf/boost.git',
        major_version=1))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='cpuid',
        git_repository='github.com/steinwurf/cpuid.git',
        major_version=3))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='fifi',
        git_repository='github.com/steinwurf/fifi.git',
        major_version=17))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='kodo',
        git_repository='github.com/steinwurf/kodo.git',
        major_version=23))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='platform',
        git_repository='github.com/steinwurf/platform.git',
        major_version=1))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='recycle',
        git_repository='github.com/steinwurf/recycle.git',
        major_version=1))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='sak',
        git_repository='github.com/steinwurf/sak.git',
        major_version=14))

    bundle.add_dependency(opt, resolve.ResolveGitMajorVersion(
        name='waf-tools',
        git_repository='github.com/steinwurf/waf-tools.git',
        major_version=2))

    opt.load('wurf_configure_output')
    opt.load('wurf_dependency_bundle')
    opt.load('wurf_tools')


def configure(conf):
    conf.load('wurf_dependency_bundle')
    conf.load('wurf_tools')

    conf.load_external_tool('install_path', 'wurf_install_path')
    conf.load_external_tool('mkspec', 'wurf_cxx_mkspec_tool')
    conf.load_external_tool('project_gen', 'wurf_project_generator')

    recurse_helper(conf, 'boost')
    recurse_helper(conf, 'cpuid')
    recurse_helper(conf, 'fifi')
    recurse_helper(conf, 'kodo')
    recurse_helper(conf, 'platform')
    recurse_helper(conf, 'recycle')
    recurse_helper(conf, 'sak')


def build(bld):
    bld.load('wurf_dependency_bundle')

    recurse_helper(bld, 'boost')
    recurse_helper(bld, 'cpuid')
    recurse_helper(bld, 'fifi')
    recurse_helper(bld, 'kodo')
    recurse_helper(bld, 'platform')
    recurse_helper(bld, 'recycle')
    recurse_helper(bld, 'sak')

    bld.recurse('encrypted_payload/')
    bld.recurse('p_coding/')
    bld.recurse('p_coding_recode/')
    bld.recurse('spoc/')
    bld.recurse('spoc_recode/')

    
