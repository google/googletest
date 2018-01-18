// Pipeline definition for the build steps of AR.
//
// Copyright: 2017 Ditto Technologies. All Rights Reserved.
// Author: Frankie Li, Daran He, John Inacay

// TODO: Need to migrate to a standardize Debian package deployment script.

properties([[
  $class: 'BuildDiscarderProperty',
  strategy: [
    $class: 'LogRotator',
    artifactDaysToKeepStr: '',
    artifactNumToKeepStr: '',
    daysToKeepStr: '',
    numToKeepStr: '5'
  ]
]]);

@Library('jenkins-shared-library@release/v1.0') _

def GIT_CREDENTIALS_ID = 'dittovto-buildbot'

def BUILD_CONFIGS = [
    'ubuntu-16-04' : [
        'docker_file'   : 'Dockerfile.xenial',
        'apt_prod_repo' : '3rdparty-16.04',
        'apt_test_repo' : '3rdparty-16.04-staging',
        'dist'          : 'xenial',
    ],
    'ubuntu-14-04' : [
        'docker_file'   : 'Dockerfile.trusty',
        'apt_prod_repo' : '3rdparty-14.04',
        'apt_test_repo' : '3rdparty-14.04-staging',
        'dist'          : 'trusty',
    ]
]

node('build && docker') {
  BUILD_CONFIGS.each { platform, build_config ->
    dir(platform) {
      stage("Checking out ${platform}") {
        git_info = ditto_git.checkoutRepo()
      }

      stage("Building and publishing ${platform} dev revision") {
        version = ditto_deb.getAndValidateVersion()
        revision = ditto_deb.buildDevRevisionString(git_info.commit)

        image_name =
          ditto_utils.buildDockerImageName(git_info.repo_name, platform)
        ditto_deb.buildInsideDocker(image_name, build_config.docker_file)
        ditto_deb.generatePackageInsideDocker(image_name, version, revision)
        ditto_deb.publishPackageToS3(build_config.apt_test_repo,
                                     build_config.dist)
      }

      stage("Installing from ${platform} repo and test") {
        ditto_deb.installPackageInsideDocker(
          image_name, build_config.apt_test_repo,
          build_config.dist, version, revision)
      }
    }
  }
}

stage("Tag and deploy?") {
  deploy_mode = "SKIP"
  if (git_info.is_release_branch) {
    ditto_utils.checkVersionInReleaseBranchName(git_info.branch, version)
    deploy_mode = input(
      message: "User input required",
      parameters: [
        choice(
          name: "Deploy \"${version}\" at hash \"${git_info.commit}\"?",
          choices: [ "SKIP", "RC", "RELEASE" ].join("\n"))])
  }
}

node('build && docker') {
  stage("Building and publishing to rc or release") {
    if (!(deploy_mode == "RC" || deploy_mode == "RELEASE")) return;

    // Do this only once inside a git directory.
    BUILD_CONFIGS.any { platform, build_config ->
      dir(platform) {
        if (deploy_mode == "RC") {
          new_rc_number = ditto_git.calcRcNumber(version)
          tag = ditto_git.getRcTag(version, new_rc_number)
          revision = ditto_deb.buildRcRevisionString(new_rc_number)
        } else if (deploy_mode == "RELEASE") {
          tag = ditto_git.getReleaseTag(version)
          revision = ditto_deb.buildReleaseRevisionString()
        }
        ditto_git.pushTag(tag, GIT_CREDENTIALS_ID)
        return true
      }
    }

    BUILD_CONFIGS.each { platform, build_config ->
      dir(platform) {
        image_name =
          ditto_utils.buildDockerImageName(git_info.repo_name, platform)
        apt_repo_to_publish = deploy_mode == "RC" ?
          build_config.apt_test_repo : build_config.apt_prod_repo

        ditto_deb.generatePackageInsideDocker(image_name, version, revision)
        ditto_deb.publishPackageToS3(apt_repo_to_publish, build_config.dist)
      }
    }
  }

  stage("Cleaning up") {
    deleteDir()
  }
}
