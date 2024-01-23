pipeline {
    agent {
        label 'Linux_Slave_1'
    }

    stages {
        stage('Checkout') {
            steps {
                script{
                // Assuming your code is in a Git repository
                cleanWs()
                sh "git clone https://github.com/sushmitha-ielektron/googletestorg.git"
            }
        } 
        }

        stage('Build') {
            steps {
                script {
                    // Compile sample1.cc
                    //sh "cd /apps/opt/workspace/googletest/googletest/samples"
                    sh '''
                    cd googletestorg
                    cmake .
                    make
                    '''
                }
            }
        }

        stage('Test') {
            steps {
                script {
                    // Run the tests and generate an XML report
                    // sh "./sample1_unittest --gtest_output=xml:test_results.xml"
                    sh '''
                    pwd
                    cd googletestorg/googletest/samples
                    pwd
                    g++ -c sample1.cc -o sample1.o

                    #Compile sample1_unittest.cc
                    g++ -c -I${GTEST_HOME}/include sample1_unittest.cc -o sample1_unittest.o

                    #Link the object files
                    g++ sample1.o sample1_unittest.o -o sample1_unittest -L${GTEST_HOME}/lib -lgtest -lgtest_main -pthread
                    ./sample1_unittest --gtest_output=xml:test_results.xml
                    '''
                }
            }
        }

        /*stage('Publish Results') {
            steps {
                junit 'test_results.xml'
            }
            }*/
    }
}


 
