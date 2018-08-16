This is a minimal example of an issue with blitting framebuffers while GL_FRAMEBUFFER_SRGB is enabled.

My testing machine runs Windows 10 with an Nvidia 1060 (drivers: 398.82).

# Description
It seems that `GL_FRAMEBUFFER_SRGB` influences framebuffer depth blits. When it is enabled, depth blits from both SRGB and non-SRGB framebuffers result in different depth values in the target of the blit (`GL_DRAW_FRAMEBUFFER`).

If the blit is done in two steps (two calls to `glFramebufferBlit` with each `GL_COLOR_BUFFER_BIT` and `GL_DEPTH_BUFFER_BIT`), the problem persists.

Only if `GL_FRAMEBUFFER_SRGB` is disabled before the `GL_DEPTH_BUFFER_BIT`, the depth buffer is copied as-is.

# Screenshots
### Correct Image
If `GL_FRAMEBUFFER_SRGB` is disabled during the depth blit, the image will be correct and look like this:
![correct](https://github.com/pfirsich/depth_blit_issue/blob/master/screenshots/correct.png)

### Incorrect Image
If `GL_FRAMEBUFFER_SRGB` is enabled during the depth blit, the image *should* not be affected and look like the correct image, but it looks like this:
![incorrect](https://github.com/pfirsich/depth_blit_issue/blob/master/screenshots/incorrect.png)
