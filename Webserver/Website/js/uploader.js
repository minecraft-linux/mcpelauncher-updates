function UploadFiles() {
    var files = document.getElementById("fileupload").files;
    for (var i = 0; i < files.length; i++) {
        var file = files[i];
        var request = new XMLHttpRequest();
        request.open("POST", "/upload");
        request.setRequestHeader("FileName", file.Name !== undefined ? file.Name : file.fileName);
        request.send(file);
    }
}