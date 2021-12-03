class MyClass {
    constructor() {
        this.message = '';
        rivets.bind($('body'), { data: this });
    }

    async initModule() {


        let files = ["bootRom.gb", "tetris.gb"];

        for (let i = 0; i < files.length; i++) {
            let file = files[i];
            let responseText = await $.ajax({
                url: './' + file,
                beforeSend: function (xhr) {
                    xhr.overrideMimeType("text/plain; charset=x-user-defined");
                }
            });
            console.log(file,responseText.length);

            
            FS.createDataFile(
                "/", // folder 
                file, // filename
                responseText, // content
                true, // read
                true // write
            );
        }

        
    }


    print(text){
        if (text.startsWith("\"")){
            let formattedText = text.replace(/['"]+/g, '');
            formattedText = formattedText.replace(/[';]+/g, ' - ');
            this.message += formattedText + "<br>";
            console.log(text);
        }
        else{
            console.log(text);
        }
    }

    async btnClick() {
        Module.callMain(["bootRom.gb", "tetris.gb"]);
    }
}
let myClass = new MyClass();

window["Module"] = {
    onRuntimeInitialized: myClass.initModule,
    print: (text) => myClass.print(text),
    // printErr: (text) => myClass.print(text)
}