TEMPLATE = lib
CONFIG += staticlib create_prl

CONFIG += c++11

load(configure)
QMAKE_CONFIG_TESTS_DIR = $$PWD/../config.tests
CONFIG += recheck

DESTDIR = $$_PRO_FILE_PWD_
OBJECTS_DIR = $$_PRO_FILE_PWD_
MOC_DIR = $$_PRO_FILE_PWD_

defineTest(libraryFeature) {
  # Used for printing & identifying what feature this function should check for
  prettyName = $$1
  # Whether this feature is a platform requirement
  isRequired = $$2
  # Suffix for use-variable, CONFIG += use_<useSuffix>
  useSuffix = $$3
  # A list of test names to execute in order. If any succeeds, subsequent tests
  # are ignored and `successFunction` is called. If none succeeds, error output is generated and
  # configuration is stopped.
  testNames_var = $$4
  testNames = $$eval($$testNames_var)

  if(!isEmpty(isRequired):$$isRequired): dependency = "required"
  else:CONFIG(use_$$useSuffix): dependency = "enabled"
  else: dependency = "detected"

  for(TEST, testNames) {
    qtCompileTest($$TEST)
    if(config_$$TEST) {
      message("["$$dependency"]" $$prettyName "support enabled")
      return(true)
    }
  }

  !equals(dependency, "detected") {
    error($$prettyName "support is" $$dependency "but tests failed! Check" $$_PRO_FILE_PWD_"/config.log for more details!")
  }
  return(false)
}
