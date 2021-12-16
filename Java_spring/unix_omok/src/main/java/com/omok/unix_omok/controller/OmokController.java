package com.omok.unix_omok.controller;


import com.omok.unix_omok.Service.OmokDataService;
import com.omok.unix_omok.model.OmokData;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.util.List;

@Controller
public class OmokController {
    private final OmokDataService omokDataService;

    public OmokController(OmokDataService omokDataService) {
        this.omokDataService = omokDataService;
    }

    @Autowired

    @GetMapping("/")
    public String home(Model model) {
        System.out.println("test2");
        List<OmokData> omokDataList = omokDataService.findList();
        model.addAttribute("omokDatas", omokDataList); //model을 통째로 넘긴다.
        System.out.println("test");
        return "home";
    }

    @GetMapping("/test")
    public String test() {
        return "test";
    }
}