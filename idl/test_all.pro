PRO test_mia
  a = test_echo()
  a = a + filter()
  a = a + filter3d()
  a = a + nrreg2d()
  PRINT, a, ' tests of 4 succeeded'
END
