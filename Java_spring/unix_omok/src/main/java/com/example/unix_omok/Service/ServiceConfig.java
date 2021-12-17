package com.example.unix_omok.Service;

import com.example.unix_omok.repository.OmokDataRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class ServiceConfig {

    private OmokDataRepository omokDataRepository;

    @Autowired
    public ServiceConfig(OmokDataRepository omokDataRepository) {
        this.omokDataRepository = omokDataRepository;
    }

    @Bean
    public OmokDataService omokDataService() {
        return new OmokDataService(omokDataRepository);
    }
}
