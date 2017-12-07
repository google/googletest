// Pipeline definition for the build steps of AR.
//
// Copyright: 2017 Ditto Technologies. All Rights Reserved.
// Author: Frankie Li
// TODO - Need to migrate to a standardize Debian package deployment script.


xenial_image = 'ubuntu16-build-env'
xenial_docker = 'Dockerfile.xenial'
trusty_image = 'ubuntu14-build-env'
trusty_docker = 'Dockerfile.trusty'
build_vars = [['ubuntu16-build-env', 'Dockerfile.xenial', '3rdparty-16.04'],
              ['ubuntu14-build-env', 'Dockerfile.trusty', '3rdparty-14.04']]


node('build && docker') {

    def is_rc = false
    def is_release = false

    // Set max number of builds to keep to 5.
    properties([[$class: 'BuildDiscarderProperty', strategy: [$class: 'LogRotator', artifactDaysToKeepStr: '', artifactNumToKeepStr: '', daysToKeepStr: '', numToKeepStr: '5']]]);

    stage('Checkout') {
        // Pull the code from the repo. `checkout` is a special Jenkins cmd.
        def scm_vars = checkout scm
        def git_tag = getGitTag()
        def git_branch = getGitBranch()

        if(git_tag) {
            is_rc = (git_tag.indexOf("-rc") >= 0)
            is_release = (git_tag.indexOf("release") >= 0)
        }
        echo "Current branch is: ${git_branch}, current tag is: ${git_tag}"
        echo "Current tag is a RC tag: ${is_rc}"
        echo "Current tag is a Release tag: ${is_release}"
    }

    for (build_var in build_vars) {

        stage('Prepare build env: ' + build_var[0]) {
            buildImage(build_var[0], build_var[1])

        }

        stage('Build: ' + build_var[0]) {

            def USER_ID = sh (
                script: 'id -u',
                returnStdout: true
            ).trim()
            def GROUP_ID = sh (
                script: 'id -g',
                returnStdout: true
            ).trim()

            withEnv(['USER_ID=${USER_ID}','GROUP_ID=${GROUP_ID}',
                     'RELEASE_KEYSTORE=keystore.jks',
                     'RELEASE_KEY_ALIAS=demoapp',
                     'RELEASE_STORE_PASSWORD=ditto1',
                     'RELEASE_KEY_PASSWORD=ditto1']) {
                docker.image(build_var[0]).inside {
                    sh('./run_build.sh')
                }
            }
        }

        stage('ArchiveArtifacts ' + build_var[0]) {
            archiveArtifacts(artifacts: 'build/*.deb')
        }

        stage('Publish ' + build_var[0]) {
            withAWS(credentials:'package-uploads') {
                sh('./publish.sh ' + build_var[2])
            }
    	
        }
    }

}

def getGitHash() {
    return sh(script: "git log -n1 --pretty='%h'", returnStdout: true).trim()
}

def getGitTag() {
    def hash = getGitHash()
    def tag = ""
    try {
        tag = sh(script: "git describe --exact-match --tags ${hash}", returnStdout: true).trim()
    } catch (e) {
      echo "No current tag."
    }
    return tag
}

def getGitBranch() {
    return sh(script: "git rev-parse --abbrev-ref HEAD", returnStdout: true).trim()
}

def buildImage(build_env, docker_file) {
    // We're checking to see if an old image exists. If so, delete it to
    // reduce total space usage.
    def OLD_IMAGE = sh (script: 'docker images -q ' + build_env,
                        returnStdout: true)
    echo "Old docker image: ${OLD_IMAGE}"

    docker.build(build_env, "-f " + docker_file + " .")

    def NEW_IMAGE = sh (script: 'docker images -q ' + build_env,
                        returnStdout: true)

    echo "New docker image: ${NEW_IMAGE}"

    if (OLD_IMAGE.length() > 0 && OLD_IMAGE != NEW_IMAGE) {
        def children = sh(script: "docker images --filter 'dangling=true' -q --no-trunc", returnStdout: true)
        echo "Removing children: ${children}"
        sh("docker rmi ${children}")
        echo "Removing old image: ${OLD_IMAGE}"
        sh("docker rmi ${OLD_IMAGE}")
    }
}
