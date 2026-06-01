object WebModule1: TWebModule1
  Actions = <
    item
      Default = True
      MethodType = mtGet
      Name = 'DefaultHandler'
      PathInfo = '/'
      OnAction = WebModule1DefaultHandlerAction
    end
    item
      Name = 'PingAction'
      PathInfo = '/ping'
      OnAction = WebModule1PingActionAction
    end>
  Height = 383
  Width = 1503
  PixelsPerInch = 144
  object WebStencilsEngine1: TWebStencilsEngine
    Dispatcher = WebFileDispatcher1
    PathTemplates = <>
    Left = 993
    Top = 37
  end
  object WebStencilsProcessor1: TWebStencilsProcessor
    Engine = WebStencilsEngine1
    Left = 993
    Top = 100
  end
  object WebFileDispatcher1: TWebFileDispatcher
    WebFileExtensions = <
      item
        MimeType = 'text/html'
        Extensions = 'html;htm'
      end
      item
        MimeType = 'text/css'
        Extensions = 'css'
      end
      item
        MimeType = 'application/javascript'
        Extensions = 'js'
      end
      item
        MimeType = 'image/jpeg'
        Extensions = 'jpeg;jpg'
      end
      item
        MimeType = 'image/png'
        Extensions = 'png'
      end>
    WebDirectories = <
      item
        DirectoryAction = dirInclude
        DirectoryMask = '*'
      end
      item
        DirectoryAction = dirExclude
        DirectoryMask = '\templates\*'
      end>
    RootDirectory = 'wwwroot'
    VirtualPath = '/'
    Left = 997
    Top = 173
  end
end
