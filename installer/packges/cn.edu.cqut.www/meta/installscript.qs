function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut",
                               "@TargetDir@/ObjectTrackingDemo.exe",
                               "@DesktopDir@/ObjectTrackingDemo.lnk",
                               "workingDirectory=@TargetDir@",
                               "description=Open ObjectTrackingDemo"
                               );
    }
}
