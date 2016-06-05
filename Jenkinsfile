node {
  stage 'Stage Checkout'

  checkout scm
  sh 'git submodule update --init'  

  stage 'Stage Build'

  // branch name from Jenkins environment variables
  echo "My branch is: ${env.BRANCH_NAME}"

  def flavor = flavor(env.BRANCH_NAME)
  echo "Building flavor ${flavor}"

  sh "make docker-build"

  stage 'Stage Archive'
  // tell Jenkins to archive the debs
  // ..

  stage 'Stage Upload'
  // ...
}

@NonCPS
def flavor(branchName) {
  def matcher = (env.BRANCH_NAME =~ /([a-z_]+)/)
  assert matcher.matches()
  matcher[0][1]
}
