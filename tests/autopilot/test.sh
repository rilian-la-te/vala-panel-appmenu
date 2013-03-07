#!/bin/sh

autopilot run tests.test_gedit.GeditTestCase.test_file_new
autopilot run tests.test_gedit.GeditTestCase.test_file_close
autopilot run tests.test_gedit.GeditTestCase.test_file_quit
autopilot run tests.test_gedit.GeditTestCase.test_edit_undo
autopilot run tests.test_gedit.GeditTestCase.test_view_toolbar
autopilot run tests.test_gedit.GeditTestCase.test_documents_untitled_document
autopilot run tests.test_gedit.GeditTestCase.test_ctrl_n
